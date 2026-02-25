

#include "SerializedObject.h"

#include <sk/Assets/Asset.h>
#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Containers/String.h>

#include <charconv>

using namespace sk;


namespace
{
    auto get_meta_value( simdjson::ondemand::object _object ) -> std::pair< cUUID, cWeak_Ptr< cAsset_Meta > >
    {
        auto& manager = cAsset_Manager::get();

        auto uuid_str = _object.find_field( "uuid" ).get_string();
        cUUID uuid;
        if( uuid_str.has_value() && uuid_str.value().size() > 32 )
            uuid = cUUID::FromString( uuid_str.value() );
        else
            uuid = cUUID::kInvalid;

        cWeak_Ptr< cAsset_Meta > meta;
        if( uuid == cUUID::kInvalid )
            meta = nullptr;
        else
            meta = manager.getAsset( uuid );

        if( meta == nullptr )
        {
            _object.reset();
            auto name = _object.find_field( "name" ).get_string().value();
            _object.reset();
            for( auto& asset : manager.GetAssetsByPath( _object.find_field( "path" ).get_string().value() ) )
            {
                if( asset->GetName().view() == name )
                {
                    meta = asset;
                    break;
                }
            }
        }

        return std::make_pair( uuid, meta );
    }

    auto make_asset_path( const cAsset_Meta& _meta )
    {
        auto path = std::string{ _meta.GetPath() };
        path += '.';
        path.append( _meta.GetExtension() );

        std::ranges::replace( path, '\\', '/' );

        return path;
    }
} // ::

Serialization::cResources::cResources( simdjson::ondemand::object _object )
{
    auto& global_types = Reflection::cType_Manager::get().GetTypes();

    for( auto field : _object )
    {
        if( field.key() == "types" )
        {
            for( auto array = field.value().get_array().value(); auto element : array )
            {
                auto id = element.get_object().find_field( "id" ).get_uint64().value();
                if( auto itr = global_types.find( id ); itr != global_types.end() )
                    types.emplace( id, itr->second );
            }
        }
        else if( field.key() == "assets" )
        {
            for( auto array = field.value().get_array().value(); auto element : array )
            {
                if( auto [ uuid, meta ] = get_meta_value( element.get_object() ); meta != nullptr )
                    assets.emplace( uuid, meta );
            }
        }
    }
}

void Serialization::cResources::StoreAsset( const cWeak_Ptr< cAsset_Meta >& _asset_meta )
{
    if( !_asset_meta.is_valid() )
        return;

    assets.emplace( _asset_meta->GetUUID(), _asset_meta );
}

void Serialization::cResources::StoreType( const type_info_t& _type )
{
    if( _type == nullptr )
        return;

    types.emplace( _type->hash, _type );
}

auto Serialization::cResources::GetAsset( const cUUID& _id ) -> cWeak_Ptr< cAsset_Meta >
{
    if( const auto itr = assets.find( _id ); itr != assets.end() )
        return itr->second;
    return nullptr;
}

auto Serialization::cResources::GetType( const uint64_t _id ) -> type_info_t
{
    if( const auto itr = types.find( _id ); itr != types.end() )
        return itr->second;
    return nullptr;
}

bool Serialization::cResources::IsEmpty() const
{
    return assets.empty() && types.empty();
}

auto Serialization::cResources::CreateJSON() -> std::string_view
{
    builder.start_object();

    builder.escape_and_append_with_quotes( "types" );
    builder.append_colon();
    builder.start_array();
    auto next_type = types.begin();
    for( const auto& type : types | std::views::values )
    {
        builder.start_object();
        builder.append_key_value( "id", type->hash.value() );
        builder.append_comma();
        builder.append_key_value( "name", type->name );
        builder.end_object();

        if( ++next_type != types.end() )
            builder.append_comma();
    }
    builder.end_array();
    builder.append_comma();

    builder.escape_and_append_with_quotes( "assets" );
    builder.append_colon();
    builder.start_array();
    auto next_asset = assets.begin();
    for( const auto& asset : assets | std::views::values )
    {
        builder.start_object();
        builder.append_key_value( "uuid", asset->GetUUID().ToString() );
        builder.append_comma();
        builder.append_key_value( "name", asset->GetName() );
        builder.append_comma();
        builder.append_key_value( "path", make_asset_path( *asset ) );
        builder.end_object();

        if( ++next_asset != assets.end() )
            builder.append_comma();
    }
    builder.end_array();

    builder.end_object();

    return builder;
}

cSerializedObject::cSerializedObject( const type_info_t _serialized_type, const size_t _element_count )
: m_serialized_type_( _serialized_type )
, m_element_count_( _element_count )
, m_json_builder_( Math::ceilToPow2( m_serialized_type_->size ) )
{}

cSerializedObject::cSerializedObject() = default;

cSerializedObject::cSerializedObject( simdjson::ondemand::object _object, Serialization::cResources* _resources )
{
    const bool owns_resources = _resources == nullptr;

    if( owns_resources )
    {
        if( auto resource_field = _object.find_field_unordered( ":resources:" ); resource_field.has_value() )
        {
            m_resources_ = new Serialization::cResources( resource_field.value().get_object() );
        }
        else
            m_resources_ = new Serialization::cResources();
    }
    else
        m_resources_ = _resources;

    _object.reset();

    for( auto field : _object )
    {
        if( field.error() )
        {
            SK_BREAK;
            break;
        }
        // The JSON should always be in this order.
        if( auto key = field.key().value(); key == ":type:" )
        {
            if( auto value = field.value(); !value.is_null() )
                m_serialized_type_ = m_resources_->GetType( value.get_uint64() );
        }
        else if( key == ":bases:" ) // The object will not contain any base if there isn't any.
        {
            for( auto element : field.value().get_array() )
                m_bases_.emplace_back( element.get_object(), m_resources_ );
        }
        else if( key[ 0 ] != ':' ) // Filter out any extra internal fields.
        {
            // Handle data members
            sValueInfo info{};
            size_t length = 0;
            size_t end    = 0;
            for( auto start = key.raw(); *start != ':'; start++ )
                length++;
            for( auto start = key.raw(); *start != '"'; start++ )
                end++;

            info.name           = std::string_view{ key.raw(), length };
            info.json_safe_name = info.name;
            info.value_index    = m_values_.size();

            // We'll only need the inline info for the element.
            _handleJsonElement( field.value().value(), std::string_view{ key.raw() + length, end - length } );

            m_info_.emplace_back( std::move( info ) );
        }
    }

    if( owns_resources )
    {
        delete m_resources_;
        m_resources_ = nullptr;
    }
}

namespace
{
    // Unknown/SerializableObject
    template< class Ty >
    constexpr auto kTypeString = std::string{};
    template<>
    constexpr auto kTypeString< cWeak_Ptr< cAsset_Meta > > = std::string{ "a:" };
    template<>
    constexpr auto kTypeString< bool > = std::string{ "b:" };
    template<>
    constexpr auto kTypeString< int64_t > = std::string{ "i:" };
    template<>
    constexpr auto kTypeString< cVector2i64 > = std::string{ "2i:" };
    template<>
    constexpr auto kTypeString< cVector3i64 > = std::string{ "3i:" };
    template<>
    constexpr auto kTypeString< cVector4i64 > = std::string{ "3i:" };
    template<>
    constexpr auto kTypeString< uint64_t > = std::string{ "u:" };
    template<>
    constexpr auto kTypeString< cVector2u64 > = std::string{ "2u:" };
    template<>
    constexpr auto kTypeString< cVector3u64 > = std::string{ "3u:" };
    template<>
    constexpr auto kTypeString< cVector4u64 > = std::string{ "4u:" };
    template<>
    constexpr auto kTypeString< double > = std::string{ "d:" };
    template<>
    constexpr auto kTypeString< cVector2d > = std::string{ "2d:" };
    template<>
    constexpr auto kTypeString< cVector3d > = std::string{ "3d:" };
    template<>
    constexpr auto kTypeString< cVector4d > = std::string{ "4d:" };
    template<>
    constexpr auto kTypeString< std::string > = std::string{ "s:" };

    enum class eValueType : uint_fast8_t
    {
        // :i:
        kInt,
        // :2i:
        kInt2,
        // :3i:
        kInt3,
        // :4i:
        kInt4,
        // :u:
        kUInt,
        // :2u:
        kUInt2,
        // :3u:
        kUInt3,
        // :4u:
        kUInt4,
        // :d:
        kDouble,
        // :2d:
        kDouble2,
        // :3d:
        kDouble3,
        // :4d:
        kDouble4,
        // :s:
        kString,
        // :b:
        kBoolean,
        // :n:
        kNull,
        // :-:
        kUnknown,
        // :a:
        kAsset,
        // :o:
        kObject,
        // :[Size]: (Known size) / :x: (Unknown size)
        kArray,
    };

    eValueType   operator+( eValueType _a, eValueType _b ){ return static_cast< eValueType >( static_cast< uint_fast8_t >( _a ) + static_cast< uint_fast8_t >( _b ) ); }
    uint_fast8_t operator-( eValueType _a, eValueType _b ){ return static_cast< uint_fast8_t >( _a ) - static_cast< uint_fast8_t >( _b ); }

    eValueType   operator+( eValueType _a, const uint_fast8_t _b ){ return static_cast< eValueType >( static_cast< uint_fast8_t >( _a ) + _b ); }
    uint_fast8_t operator-( eValueType _a, const uint_fast8_t _b ){ return static_cast< uint_fast8_t >( _a ) - _b; }

    auto get_vector_size( const eValueType _base_type, const char& _type_end ) -> eValueType
    {
        // We're reading
        //  v
        // [N][_type_end]:
        // In a correct scenario it'll be '2', '3', '4' or ':'
        switch( *( &_type_end - 1 ) )
        {
        case ':': return _base_type;
        case '2': return _base_type + 1;
        case '3': return _base_type + 2;
        case '4': return _base_type + 3;
        default: SK_BREAK; return _base_type;
        }
    }

    auto get_value_type( const std::string_view& _key ) ->  eValueType
    {
        switch( const auto& c = _key[ _key.length() - 2 ]; c )
        {
        // Int
        case 'i': return get_vector_size( eValueType::kInt, c );

        // UInt
        case 'u': return get_vector_size( eValueType::kUInt, c );

        // Double
        case 'd': return get_vector_size( eValueType::kDouble, c );

        // String
        case 's': return eValueType::kString;

        // Bool
        case 'b': return eValueType::kBoolean;

        // Null
        case 'n': return eValueType::kNull;

        // Unknown
        case '-': return eValueType::kUnknown;

        // Asset
        case 'a': return eValueType::kAsset;

        // Array
        case 'x':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': return eValueType::kArray;

        // Object
        case 'o': return eValueType::kObject;

        default: SK_BREAK; return eValueType::kUnknown;
        }
    }

    template< class Ty >
    auto handle_vector_value( simdjson::ondemand::array _array, uint_fast8_t _size )
    {
        using value_t = cSerializedObject::value_t;

        ++_size;

        std::vector< Ty > elements( _size );
        for( size_t i = 0; auto element : _array )
            elements[ i++ ] = element.get< Ty >();

        switch( _size )
        {
        case 2:  return value_t{ Math::cVector2< Ty >{ elements.data() } };
        case 3:  return value_t{ Math::cVector3< Ty >{ elements.data() } };
        case 4:  return value_t{ Math::cVector4< Ty >{ elements.data() } };
        default: SK_BREAK;
        }

        return value_t{ std::monostate{} };
    }

    // Returns size, element type
    auto get_array_info( const std::string_view& _key ) -> std::pair< size_t, std::string_view >
    {
        if( _key[ _key.length() - 2 ] == 'x' )
            return std::make_pair( std::numeric_limits< size_t >::max(), std::string_view{ _key.data(), _key.length() - 3 } );

        const auto start = _key.find_last_of( ':', _key.length() - 2 ) + 1;
        const auto end   = _key.length() - 1;

        uint32_t value;
        if( std::from_chars( _key.data() + start, _key.data() + end, value ).ec == std::errc{} )
            return std::make_pair( value, std::string_view{ _key.data(), start } );

        return std::make_pair( std::numeric_limits< size_t >::max(), std::string_view{ _key.data(), start - 1 } );
    }

    auto handle_json_array_value( const simdjson::ondemand::array& _array, const std::string_view& _key, Serialization::cResources* _resources ) -> cSerializedObject::value_t
    {
        auto [ size, element_type ] = get_array_info( _key );
        return cSerializedObject( _array, size, element_type, _resources );
    }

    auto handle_json_element_( simdjson::ondemand::value& _element, const eValueType _type, const std::string_view& _key = {}, Serialization::cResources* _resources = nullptr ) -> cSerializedObject::value_t
    {
        switch( _type )
        {
        case eValueType::kInt:     return _element.get_int64();
        case eValueType::kInt2:
        case eValueType::kInt3:
        case eValueType::kInt4:    return handle_vector_value< int64_t >( _element, _type - eValueType::kInt );
        case eValueType::kUInt:    return _element.get_uint64();
        case eValueType::kUInt2:
        case eValueType::kUInt3:
        case eValueType::kUInt4:   return handle_vector_value< uint64_t >( _element, _type - eValueType::kUInt );
        case eValueType::kDouble:  return _element.get_double();
        case eValueType::kDouble2:
        case eValueType::kDouble3:
        case eValueType::kDouble4: return handle_vector_value< double >( _element, _type - eValueType::kDouble );
        case eValueType::kString:  return std::string{ _element.get_string().value() };
        case eValueType::kBoolean: return _element.get_bool();
        case eValueType::kNull:
        case eValueType::kUnknown: return std::monostate{};
        case eValueType::kAsset:   return _resources->GetAsset( cUUID::FromString( _element.get_string() ) );
        case eValueType::kArray:   return handle_json_array_value( _element.get_array(), _key, _resources );
        case eValueType::kObject:  return cSerializedObject( _element.get_object(), _resources );
        }
        return std::monostate{};
    }

    auto handle_json_element_( simdjson::ondemand::value _element, const std::string_view& _key, Serialization::cResources* _resources ) -> cSerializedObject::value_t
    {
        return std::move( handle_json_element_( _element, get_value_type( _key ), _key, _resources ) );
    }

    template< class Ty >
    auto handle_json_array( simdjson::ondemand::array _array, const size_t _length, const eValueType _type, const std::string_view& _key = {}, Serialization::cResources* _resources = nullptr ) -> cSerializedObject::array_t
    {
        auto buffer = SK_NEW( Ty, _length );
        for( auto val = buffer; auto element : _array )
            *val++ = std::get< Ty >( handle_json_element_( element.value(), _type, _key, _resources ) );

        return buffer;
    }
} // ::

cSerializedObject::cSerializedObject( simdjson::ondemand::array _array, size_t _length, const std::string_view& _key, Serialization::cResources* _resources )
{
    if( _length == std::numeric_limits< size_t >::max() )
        _length = _array.count_elements();

    m_element_count_ = _length;
    if( m_element_count_ == 0 )
        return;

    switch( const auto type = get_value_type( _key ); type )
    {
    case eValueType::kInt:     m_element_data_ = handle_json_array< int64_t >( _array, _length, type );     break;
    case eValueType::kInt2:    m_element_data_ = handle_json_array< cVector2i64 >( _array, _length, type ); break;
    case eValueType::kInt3:    m_element_data_ = handle_json_array< cVector3i64 >( _array, _length, type ); break;
    case eValueType::kInt4:    m_element_data_ = handle_json_array< cVector4i64 >( _array, _length, type ); break;
    case eValueType::kUInt:    m_element_data_ = handle_json_array< uint64_t >( _array, _length, type );    break;
    case eValueType::kUInt2:   m_element_data_ = handle_json_array< cVector2u64 >( _array, _length, type ); break;
    case eValueType::kUInt3:   m_element_data_ = handle_json_array< cVector3u64 >( _array, _length, type ); break;
    case eValueType::kUInt4:   m_element_data_ = handle_json_array< cVector4u64 >( _array, _length, type ); break;
    case eValueType::kDouble:  m_element_data_ = handle_json_array< int64_t >( _array, _length, type );     break;
    case eValueType::kDouble2: m_element_data_ = handle_json_array< cVector2d >( _array, _length, type );   break;
    case eValueType::kDouble3: m_element_data_ = handle_json_array< cVector3d >( _array, _length, type );   break;
    case eValueType::kDouble4: m_element_data_ = handle_json_array< cVector4d >( _array, _length, type );   break;
    case eValueType::kString:  m_element_data_ = handle_json_array< std::string >( _array, _length, type ); break;
    case eValueType::kBoolean: m_element_data_ = handle_json_array< bool >( _array, _length, type );        break;
    case eValueType::kNull:
    case eValueType::kUnknown: m_element_data_ = std::monostate{}; break;
    case eValueType::kAsset:   m_element_data_ = handle_json_array< cWeak_Ptr< cAsset_Meta > >( _array, _length, type, _key, _resources ); break;
    case eValueType::kObject:
    case eValueType::kArray:   m_element_data_ = handle_json_array< cSerializedObject >( _array, _length, type, _key, _resources ); break;
    }
}

cSerializedObject::cSerializedObject( const cSerializedObject& _other )
: m_serialized_type_( _other.m_serialized_type_ )
, m_element_count_( _other.m_element_count_ )
, m_json_builder_( _other._hasCompletedJson() ? _other.m_json_builder_.size() : json_builder_t::DEFAULT_INITIAL_CAPACITY )
{
    if( _other.IsArray() )
    {
        std::visit( [&]< class V >( const V& _array )
        {
            if constexpr( !std::is_same_v< V, std::monostate > )
            {
                using element_t = std::remove_cvref_t< std::remove_pointer_t< V > >;

                auto ptr = SK_NEW( element_t, m_element_count_ );
                m_element_data_ = ptr;
                std::copy_n( _array, m_element_count_, ptr );
            }
        }, _other.m_element_data_ );
    }
    else
    {
        m_info_   = _other.m_info_;
        m_values_ = _other.m_values_;
        m_bases_  = _other.m_bases_;
    }
    
    if( _other._hasCompletedJson() )
        m_json_builder_.append_raw( _other.m_json_builder_ );
}

cSerializedObject::cSerializedObject( cSerializedObject&& _other ) noexcept
: m_serialized_type_( _other.m_serialized_type_ )
, m_element_count_( _other.m_element_count_ )
, m_element_data_( _other.m_element_data_ )
, m_json_builder_( std::move( _other.m_json_builder_ ) )
, m_bases_( std::move( _other.m_bases_ ) )
, m_info_( std::move( _other.m_info_ ) )
, m_values_( std::move( _other.m_values_ ) )
{
    _other.m_element_data_  = std::monostate{};
    _other.m_element_count_ = 0;
}

cSerializedObject::~cSerializedObject()
{
    Reset();
}

cSerializedObject& cSerializedObject::operator=( const cSerializedObject& _other )
{
    if( &_other == this )
        return *this;
    
    Reset();
    
    m_serialized_type_ = _other.m_serialized_type_;
    m_element_count_   = _other.m_element_count_;
    
    if( _other.IsArray() )
    {
        std::visit( [&]< class V >( const V& _array )
        {
            if constexpr( !std::is_same_v< V, std::monostate > )
            {
                using element_t = std::remove_pointer_t< V >;

                auto ptr = SK_NEW( element_t, m_element_count_ );
                m_element_data_ = ptr;
                std::copy_n( _array, m_element_count_, ptr );
            }
        }, _other.m_element_data_ );
    }
    else
    {
        m_info_   = _other.m_info_;
        m_values_ = _other.m_values_;
        m_bases_  = _other.m_bases_;
    }
    
    if( _other._hasCompletedJson() )
        m_json_builder_.append_raw( _other.m_json_builder_ );
    
    return *this;
}

cSerializedObject& cSerializedObject::operator=( cSerializedObject&& _other ) noexcept
{
    Reset();
    
    m_serialized_type_ = _other.m_serialized_type_;
    m_element_count_   = _other.m_element_count_;
    m_element_data_    = _other.m_element_data_ ;
    m_json_builder_    = std::move( _other.m_json_builder_ );
    m_info_            = std::move( _other.m_info_ );
    m_values_          = std::move( _other.m_values_ );
    m_bases_           = std::move( _other.m_bases_ );

    _other.m_element_data_  = std::monostate{};
    _other.m_element_count_ = 0;
    
    return *this;

}

bool cSerializedObject::IsArray() const
{
    return m_element_data_.index() != 0;
}

void cSerializedObject::Reset()
{
    ClearCache();
    
    m_serialized_type_ = nullptr;
    if( IsArray() )
    {
        std::visit( []< class V >( V& _array ){
            if constexpr( !std::is_same_v< V, std::monostate > )
                SK_DELETE( _array );
        }, m_element_data_ );
        m_element_count_ = 0;
        m_element_data_  = std::monostate{};
    }
    else
    {
        m_info_.clear();
        m_values_.clear();
        m_bases_.clear();
    }
}

auto cSerializedObject::CreateJSON() -> std::string_view
{
    m_json_builder_.clear();
    m_resources_ = new Serialization::cResources();
    json_builder_t temp_builder{};
    createJsonInternal( temp_builder, nullptr );

    m_json_builder_.start_object();

    if( !m_resources_->IsEmpty() )
    {
        m_json_builder_.escape_and_append_with_quotes( ":resources:" );
        m_json_builder_.append_colon();
        m_json_builder_.append_raw( m_resources_->CreateJSON() );
        m_json_builder_.append_comma();
    }
    m_json_builder_.append_raw( temp_builder );

    m_json_builder_.end_object();
    delete m_resources_;
    m_resources_ = nullptr;

    return m_json_builder_;
}

void cSerializedObject::createJsonInternal( json_builder_t& _builder, std::string* _type, Serialization::cResources* _resources )
{
    const bool owns_resources = _resources == nullptr;
    if( !owns_resources )
        m_resources_ = _resources;

    // As we're mainly going to be recreating the structure from the ground up. Only cache if the user explicitly creates the JSON for an object.
    if( !_hasCompletedJson() )
    {
        if( !IsArray() )
        {
            if( !owns_resources )
                _builder.start_object();

            // :type: - uint64_t
            if( m_serialized_type_ != nullptr )
            {
                m_resources_->StoreType( m_serialized_type_ );
                _builder.append_key_value( ":type:", m_serialized_type_->hash.value() );
                if( !m_bases_.empty() || !m_info_.empty() )
                    _builder.append_comma();
            }

            // :bases: - SerializedObject[]
            if( m_serialized_type_ != nullptr && !m_bases_.empty() )
            {
                _builder.escape_and_append_with_quotes( ":bases:" );
                _builder.append_colon();
                _builder.start_array(); // Bases start

                for( size_t i = 0; i < m_bases_.size(); i++ )
                {
                    m_bases_[ i ].createJsonInternal( _builder, nullptr, m_resources_ );
                    if( i != m_bases_.size() - 1 )
                        _builder.append_comma();
                }

                _builder.end_array(); // Bases end
                if( !m_info_.empty() )
                    _builder.append_comma();
            }

            _createJsonObject( _builder );

            if( !owns_resources )
                _builder.end_object();
        }
        else
            _createJsonArray( _builder, *_type );
    }
    else if( &_builder != &m_json_builder_ )
    {
        // This is currently broken.
        SK_BREAK;
        _builder.append_raw( m_json_builder_ );
    }

    if( !owns_resources )
        m_resources_ = nullptr;
}

auto cSerializedObject::CreateBinary() -> std::span< std::byte >
{
    // TODO: Binary export.
    return m_binary_cache_;
}

void cSerializedObject::ClearCache()
{
    m_json_builder_.clear();
}

auto cSerializedObject::GetType() const -> type_info_t
{
    return m_serialized_type_;
}

auto cSerializedObject::GetRuntimeClass() const -> class_info_t*
{
    return m_serialized_type_->as_class_info()->runtime_class;
}

auto cSerializedObject::GetBase( type_info_t _type ) -> std::optional< std::reference_wrapper< cSerializedObject > >
{
    if( const auto itr = std::ranges::find_if( m_bases_, [ &_type ]( const auto& _base ) -> bool{ return _base.m_serialized_type_ == _type; } );
        itr != m_bases_.end() )
        return std::ref( *itr );

    return std::nullopt;
}

auto cSerializedObject::ConstructClass() -> iClass*
{
    return GetRuntimeClass()->CreateSerialized( *this );
}

auto cSerializedObject::ConstructSharedClass() -> cShared_ptr< iClass >
{
    return GetRuntimeClass()->CreateSharedSerialized( *this );
}

void cSerializedObject::BeginRead( iClass* _this )
{
    m_this_ = _this;
}

auto cSerializedObject::ReadValueRaw( const cStringID& _name ) -> std::optional< std::reference_wrapper< value_t > >
{
    // TODO: Print error
    auto pretty_name = cStringID{ MakeJsonSafeName( _name ) };
    auto itr = std::ranges::find_if( m_info_, [&_name]( auto& _info ){ return _info.json_safe_name == _name; } );
    if( itr != m_info_.end() )
        return std::ref( m_values_[ itr->value_index ] );

    return std::nullopt;
}

auto cSerializedObject::GetArraySize() const -> size_t
{
    return m_element_count_;
}

void cSerializedObject::EndRead()
{
    m_this_ = nullptr;
}

auto cSerializedObject::BeginWrite( iClass* _this, const bool _reset ) -> cSerializedObject&
{
    m_this_ = _this;
    
    if( _reset )
        Reset();

    return *this;
}

auto cSerializedObject::AddBase( cSerializedObject&& _base_info ) -> cSerializedObject&
{
    m_bases_.emplace_back( std::move( _base_info ) );

    return *this;
}

auto cSerializedObject::EndWrite() -> cSerializedObject&&
{
    ClearCache();
    
    m_this_ = nullptr;
    
    for( auto& info : m_info_ )
    {
        if( ( info.flags & sValueInfo::kRealMember ) == 0 )
            continue;

        info.offset += m_raw_offset_;
    }

    return std::move( *this );
}

std::string cSerializedObject::MakeJsonSafeName( const std::string_view& _name )
{
    std::stringstream ss;

    for( auto c : _name )
    {
        bool valid = true;
        switch( c )
        {
        case '_':
        case '-':
        case '.':
            valid = true; break;
        case ' ':
            c = '_'; break;
        default:
            valid = std::isalnum( c );
        }

        if( !valid )
            continue;

        ss << c;
    }

    return ss.str();
}

void cSerializedObject::_createJsonObject( json_builder_t& _builder )
{
    // TODO: Store member variables.
    auto& builder = _builder;

    for( auto& info : m_info_ )
    {
        _handleInfo( builder, info, info.json_safe_name.string() );
        
        if( &info != &m_info_.back() )
            builder.append_comma();
    }
}

namespace
{
    void handle_value( cSerializedObject::json_builder_t& _builder, auto& _value, std::string* _type, Serialization::cResources* _resources )
    {
        auto& builder = _builder;

        if( _type != nullptr )
            *_type = kTypeString< decltype( _value ) >;

        sVisitor{
            [&]( std::monostate& )
            {
                builder.append_null();
            },
            [&]( cSerializedObject& _val )
            {
                _val.createJsonInternal( builder, _type );
            },
            [&]( const bool& _val )
            {
                builder.append( _val );
            },
            [&]( const int64_t& _val )
            {
                builder.append( _val );
            },
            [&]( const uint64_t& _val )
            {
                builder.append( _val );
            },
            [&]( const double& _val )
            {
                builder.append( _val );
            },
            [&]( const std::string& _val )
            {
                builder.escape_and_append_with_quotes( _val );
            },
            [&]< class Ty >( const Math::cVector< 2, Ty >& _vector )
            {
                builder.start_array();
                builder.append( _vector[ 0 ] );
                builder.append_comma();
                builder.append( _vector[ 1 ] );
                builder.end_array();
            },
            [&]< class Ty >( const Math::cVector< 3, Ty >& _vector )
            {
                builder.start_array();
                builder.append( _vector[ 0 ] );
                builder.append_comma();
                builder.append( _vector[ 1 ] );
                builder.append_comma();
                builder.append( _vector[ 2 ] );
                builder.end_array();
            },
            [&]< class Ty >( const Math::cVector< 4, Ty >& _vector )
            {
                builder.start_array();
                builder.append( _vector[ 0 ] );
                builder.append_comma();
                builder.append( _vector[ 1 ] );
                builder.append_comma();
                builder.append( _vector[ 2 ] );
                builder.append_comma();
                builder.append( _vector[ 3 ] );
                builder.end_array();
            },
            [&]( const cWeak_Ptr< cAsset_Meta >& _meta )
            {
                if( _meta.is_valid() )
                {
                    _resources->StoreAsset( _meta );
                    builder.append( _meta->GetUUID().ToString() );
                }
                else
                    builder.append_null();
            }
        }( _value );
    }
} // ::

void cSerializedObject::_createJsonArray( json_builder_t& _builder, std::string& _type )
{
    auto& builder = _builder;

    // We'll use our own builder to make the array values.
    m_json_builder_.start_array();

    if( m_element_count_ > 0 )
    {
        std::string element_type{};

        sVisitor visitor{
            [&]< class V > requires ( !std::is_same_v< V, std::monostate > && !std::is_same_v< V, cSerializedObject > ) ( V*& _array )
            {
                // Not nested. Will always share the same type
                for( size_t i = 0; i < m_element_count_; i++ )
                {
                    handle_value( _builder, _array[ i ], nullptr, m_resources_ );

                    if( i != m_element_count_ - 1 )
                        builder.append_comma();
                }
                _type = kTypeString< V > + std::to_string( m_element_count_ ) + ':';
            },
            [&]( cSerializedObject*& _array )
            {
                // The element is either a nested array or an object.
                if( _array->IsArray() )
                {
                    // Nested arrays SHOULD always have the same leaf type. But the nested arrays are allowed to have different sizes.
                    bool       shares_size  = true;
                    const auto nested_length = _array[ 0 ].GetArraySize();
                    handle_value( _builder, _array[ 0 ], &element_type, m_resources_ );
                    for( size_t i = 1; i < m_element_count_; i++ )
                    {
                        handle_value( _builder, _array[ i ],nullptr, m_resources_ );

                        if( shares_size && nested_length != _array[ i ].GetArraySize() )
                            shares_size = false;

                        if( i != m_element_count_ - 1 )
                            builder.append_comma();
                    }

                    if( shares_size ) // [T]:[EN]: -> [T]:[EN]:[N]
                        _type = element_type + std::to_string( m_element_count_ ) + ':';
                    else
                    {
                        // We need to start at the type
                        const auto element_size_open = element_type.find_last_of( ':', element_type.length() - 1 );
                        // [T]:[EN]: -> [T]:x:[N]:
                        _type = element_type.substr( 0, element_size_open + 1 ) + "x:" + std::to_string( m_element_count_ ) + ':';
                    }
                }
                else
                {
                    for( size_t i = 0; i < m_element_count_; i++ )
                    {
                        _array[ i ].createJsonInternal( _builder, nullptr, m_resources_ );

                        if( i != m_element_count_ - 1 )
                            builder.append_comma();
                    }
                    // o:[N]:
                    _type = "o:" + std::to_string( m_element_count_ ) + ':';
                }
            },
            []( const std::monostate& )
            {
                // This shouldn't happen.
                SK_BREAK;
            },
        };

        std::visit( visitor, m_element_data_ );
    }
    else
        _type = "n:0:";

    m_json_builder_.end_array();
    if( &_builder != &m_json_builder_ )
        _builder.append_raw( m_json_builder_ );
}

void cSerializedObject::_handleInfo( json_builder_t& _builder, const sValueInfo& _info, const std::string& _key )
{
    auto& info    = _info;
    auto& element = m_values_[ info.value_index ];

    std::visit( [ & ]< class V >( V& _value )
    {
        if constexpr( std::is_same_v< V, cSerializedObject > )
        {
            if( _value.IsArray() )
            {
                std::string type{};

                _value.m_json_builder_.clear();
                _value.m_resources_ = m_resources_;
                _value._createJsonArray( _value.m_json_builder_, type );
                _value.m_resources_ = nullptr;

                _builder.escape_and_append_with_quotes( _key + ':' + type );
                _builder.append_colon();

                _builder.append_raw( _value.m_json_builder_ );
            }
            else // It's an object so we can skip the additional checking arrays need.
            {
                _builder.escape_and_append_with_quotes( _key + ":o:" );
                _builder.append_colon();
                _value.createJsonInternal( _builder, nullptr, m_resources_ );
            }
        }
        else
        {
            _builder.escape_and_append_with_quotes( _key + ':' + kTypeString< V > );
            _builder.append_colon();
            handle_value( _builder, _value, nullptr, m_resources_ );
        }
    }, element );
}

void cSerializedObject::_handleJsonElement( const simdjson::ondemand::value& _element, const std::string_view _key )
{
    m_values_.emplace_back( handle_json_element_( _element, _key, m_resources_ ) );
}

void cSerializedObject::_writeData( const cStringID& _name, value_t&& _value )
{
    const auto json_safe_name = MakeJsonSafeName( _name.view() );

    const auto index = m_info_.size();
    m_values_.emplace_back( std::move( _value ) );

    sValueInfo info{
        .name = _name.view(),
        .json_safe_name = std::string_view{ json_safe_name },
        .offset = 0,
        .value_index = static_cast< uint32_t >( index ),
        .flags  = 0,
        .type   = nullptr,
    };

    m_info_.emplace_back( std::move( info ) );
}

auto cSerializedObject::_getValueAtOffset( type_info_t _type, size_t _offset ) -> std::optional< value_t >
{
    _offset += m_raw_offset_;
    const auto itr = std::ranges::find_if( m_info_, [&]( auto& _info )
    {
        if( _info.offset != _offset )
            return false;
        
        SK_BREAK_RET_IF( sk::Severity::kEngine,
            _info.type != _type, "Error: The type at this offset doesn't have this value", false )
        return true;
    } );
    
    SK_BREAK_RET_IF( sk::Severity::kEngine,
        itr == m_info_.end(), "Error: No value at this offset.", std::nullopt )
    
    auto& value = m_values_[ itr->value_index ];
    
    return value;
}

bool cSerializedObject::_hasCompletedJson() const
{
    // The JSON builder will never have any data unless it's completed.
    return m_json_builder_.size() > 0;
}
