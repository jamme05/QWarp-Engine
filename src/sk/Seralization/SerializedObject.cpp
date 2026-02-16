

#include "SerializedObject.h"

#include <sk/Assets/Asset.h>
#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Containers/String.h>

#include <charconv>

using namespace sk;


namespace
{
    auto get_meta_value( const simdjson::dom::object& _object ) -> cSerializedObject::value_t
    {
        auto& manager = cAsset_Manager::get();

        auto uuid_str = _object.at_key( "asset_uuid" ).get_string();
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
            meta = manager.GetAssetByPath( std::filesystem::path{ _object.at_key( "asset_path" ).get_string().value() } );

        return meta;
    }
} // ::

Serialization::cResources::cResources( simdjson::ondemand::object _object )
{
    auto& types = Reflection::cType_Manager::get().GetTypes();
}

void Serialization::cResources::StoreAsset( const cWeak_Ptr< cAsset_Meta >& _asset_id )
{

}

void Serialization::cResources::StoreType( const type_info_t& _type )
{

}

auto Serialization::cResources::GetAsset( const cUUID& _id ) -> cWeak_Ptr< cAsset_Meta >
{

}

auto Serialization::cResources::GetType( uint64_t _id ) -> type_info_t
{
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

    for( auto field : _object )
    {
        // The JSON should always be in this order.

        if( auto key = field.key().value(); owns_resources && key == ":resources:" )
            _resources = new Serialization::cResources( field.value().get_object() );
        else if( key == ":type:" )
        {
            if( auto value = field.value(); !value.is_null() )
                m_serialized_type_ = _resources->GetType( value.get_uint64() );
        }
        else if( key == ":bases:" ) // The object will not contain any base if there isn't any.
            m_bases_.emplace_back( field.value().get_object(), _resources );
        else
        {
            // Handle data members
            sValueInfo info{};
            size_t length = 0;
            for( auto start = key.raw(); *start != ':'; start++ )
                length++;

            info.name        = std::string_view{ key.raw(), length };
            info.value_index = m_values_.size();

            // We'll only need the inline info for the element.
            handle_json_element( field.value().value(), std::string_view{ key.raw() + length }, _resources );

            m_info_.emplace_back( std::move( info ) );
        }
    }
}

namespace
{

    template< class Ty >
    auto handle_vector_value( simdjson::ondemand::array _array, const std::string_view& _key )
    {
        using value_t = cSerializedObject::value_t;
        auto size = _key[ _key.length() - 3 ] - '0';

        std::vector< Ty > elements( size );
        for( size_t i = 0; auto element : _array )
            elements[ i++ ] = element.get< Ty >();

        switch( size )
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

        const auto start = _key.find_last_not_of( ':', _key.length() - 2 );
        const auto end   = _key.length() - 1;

        uint32_t value;
        if( std::from_chars( _key.data() + start, _key.data() + end, value ).ec == std::errc{} )
            return std::make_pair( value, std::string_view{ _key.data(), start - 1 } );

        return std::make_pair( std::numeric_limits< size_t >::max(), std::string_view{ _key.data(), start - 1 } );
    }

    auto handle_json_array_value( simdjson::ondemand::array _array, const std::string_view& _key, Serialization::cResources* _resources ) -> cSerializedObject::value_t
    {
        const char* type_info = &_key.back() - 1;

        // It's a normal array
        if( *type_info == 'x' || std::isdigit( *type_info ) )
        {
            auto [ size, element_type ] = get_array_info( _key );
            return cSerializedObject( _array, size, element_type, _resources );
        }
        switch( *type_info )
        {
        case 'd': return handle_vector_value< double >( _array, _key );
        case 'i': return handle_vector_value< int64_t >( _array, _key );
        case 'u': return handle_vector_value< uint64_t >( _array, _key );
        default: return std::monostate{};
        }
    }

    auto handle_json_element_( simdjson::ondemand::value _element, std::string_view _key, Serialization::cResources* _resources ) -> cSerializedObject::value_t
    {
        using element_type = simdjson::ondemand::json_type;
        using number_type  = simdjson::ondemand::number_type;

        const bool has_type = _key.back() == ':';

        switch( _element.type().value() )
        {
        case element_type::array:  return handle_json_array_value( _element.get_array().value(), _key, _resources );
        case element_type::object: return cSerializedObject( _element.get_object(), _resources );
        case element_type::number:
            if( has_type )
            {
                // Is this a type resource reference?
                if( _key[ _key.length() - 2 ] == 't' )
                {
                    // TODO: Add type info to value_t
                    break;
                }
            }
            switch( _element.get_number_type() )
            {
            case number_type::unsigned_integer:      return _element.get_uint64(); break;
            case number_type::signed_integer:        return _element.get_int64(); break;
            case number_type::floating_point_number: return _element.get_double(); break;
            default: return std::monostate{}; break; // Use null
            }
        case element_type::string:
            if( has_type )
            {
                // Is this an asset resource reference?
                if( _key[ _key.length() - 2 ] == 'a' )
                    return _resources->GetAsset( cUUID::FromString( _element.get_string() ) );
            }
            return std::string{ _element.get_string().value() };
        case element_type::boolean: return _element.get_bool();
        case element_type::null:
        case element_type::unknown: return std::monostate{};
        }
        return std::monostate{};
    }

    template< class Ty >
    auto handle_json_array( simdjson::ondemand::array _array, size_t _length, const std::string_view& _key, Serialization::cResources* _resources ) -> cSerializedObject::array_t
    {
        auto buffer = SK_NEW( Ty, _length );
        for( auto val = buffer; auto element : _array )
            *val++ = std::get< Ty >( handle_json_element_( element.value(), _key, _resources ) );

        return buffer;
    }
} // ::

cSerializedObject::cSerializedObject( simdjson::ondemand::array _array, size_t _length, const std::string_view& _key, Serialization::cResources* _resources )
{
    using element_type = simdjson::ondemand::json_type;
    using number_type  = simdjson::ondemand::number_type;

    if( _length == std::numeric_limits< size_t >::max() )
        _length = _array.count_elements();

    m_element_count_ = _length;
    if( m_element_count_ == 0 )
        return;



    switch( auto first = *_array.begin(); first.type().value() )
    {
    case element_type::array:
    {
        if(  )
        m_element_data_ = handle_json_array< cSerializedObject >( _array, _length, _key, _resources ); break;
    }
    case element_type::object: m_element_data_ = handle_json_array< cSerializedObject >( _array, _length, _key, _resources ); break;
    case element_type::number:
        switch( first.get_number_type().value() )
        {
        case number_type::floating_point_number: m_element_data_ = handle_json_array< double >( _array, _length, _key, _resources ); break;
        case number_type::signed_integer:        m_element_data_ = handle_json_array< int64_t  >( _array, _length, _key, _resources ); break;
        case number_type::unsigned_integer:      m_element_data_ = handle_json_array< uint64_t >( _array, _length, _key, _resources ); break;
        case number_type::big_integer:           break;
        }
        break;
    case element_type::string:  m_element_data_ = handle_json_array< std::string >( _array, _length, _key, _resources ); break;
    case element_type::boolean: m_element_data_ = handle_json_array< bool >( _array, _length, _key, _resources ); break;
    case element_type::unknown:
    case element_type::null: break;
    }
}

cSerializedObject::cSerializedObject( const cSerializedObject& _other )
: m_serialized_type_( _other.m_serialized_type_ )
, m_element_count_( _other.m_element_count_ )
, m_json_builder_( _other.has_completed_json() ? _other.m_json_builder_.size() : json_builder_t::DEFAULT_INITIAL_CAPACITY )
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
    
    if( _other.has_completed_json() )
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
    
    if( _other.has_completed_json() )
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
    CreateJSON( m_json_builder_ );

    return m_json_builder_;
}

void cSerializedObject::CreateJSON( json_builder_t& _builder )
{
    if( !has_completed_json() )
    {
        if( !IsArray() )
        {
            m_json_builder_.start_object();

            // Write type header.
            m_json_builder_.escape_and_append_with_quotes( "header" );
            m_json_builder_.append_colon();
            m_json_builder_.start_object(); // Header body start

            if( m_serialized_type_ != nullptr )
            {
                m_json_builder_.append_key_value( "valid_type", true );
                m_json_builder_.append_comma();
                m_json_builder_.append_key_value( "type_name",  m_serialized_type_->name );
                m_json_builder_.append_comma();
                m_json_builder_.append_key_value( "type_id",    m_serialized_type_->hash.value() );
            }
            else
                m_json_builder_.append_key_value( "valid_type", false );

            m_json_builder_.end_object(); // Header body end
            m_json_builder_.append_comma();

            if( m_serialized_type_ != nullptr && !m_bases_.empty() )
            {
                m_json_builder_.escape_and_append_with_quotes( "bases" );
                m_json_builder_.append_colon();
                m_json_builder_.start_array(); // Bases body start

                for( size_t i = 0; i < m_bases_.size(); i++ )
                {
                    m_bases_[ i ].CreateJSON( m_json_builder_ );
                    if( i != m_bases_.size() - 1 )
                        m_json_builder_.append_comma();
                }

                m_json_builder_.end_array(); // Bases body end
                m_json_builder_.append_comma();
            }

            m_json_builder_.escape_and_append_with_quotes( "data" );
            m_json_builder_.append_colon();

            create_json_object( m_json_builder_ );

            m_json_builder_.end_object();
        }
        else
            create_json_array( m_json_builder_ );
    }

    if( &_builder != &m_json_builder_ )
        _builder.append_raw( m_json_builder_ );
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

auto cSerializedObject::GetRuntimeClass() const -> class_info_t
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

auto cSerializedObject::ReadDataRaw( const cStringID& _name ) -> std::optional< std::reference_wrapper< value_t > >
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

void cSerializedObject::BeginWrite( iClass* _this, const bool _reset )
{
    m_this_ = _this;
    
    if( _reset )
        Reset();
}

void cSerializedObject::AddBase( cSerializedObject&& _base_info )
{
    m_bases_.emplace_back( std::move( _base_info ) );
}

void cSerializedObject::EndWrite()
{
    ClearCache();
    
    m_this_ = nullptr;
    
    for( auto& info : m_info_ )
    {
        if( ( info.flags & sValueInfo::kRealMember ) == 0 )
            continue;

        info.offset += m_raw_offset_;
    }
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

void cSerializedObject::create_json_object( json_builder_t& _builder )
{
    // TODO: Store member variables.
    auto& builder = _builder;
    
    builder.start_object();

    for( auto& info : m_info_ )
    {
        builder.escape_and_append_with_quotes( info.json_safe_name );
        builder.append_colon();
        
        handle_info( builder, info );
        
        if( &info != &m_info_.back() )
            builder.append_comma();
    }
    
    builder.end_object();
}

namespace
{
        void handle_value( cSerializedObject::json_builder_t& _builder, auto& _value )
    {
        auto& builder = _builder;

        sVisitor{
            [&]( std::monostate& )
            {
                builder.append_null();
            },
            [&]( cSerializedObject& _val )
            {
                _val.CreateJSON( builder );
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
                builder.start_object();
                builder.append_key_value( "x", _vector[ 0 ] );
                builder.append_comma();
                builder.append_key_value( "y", _vector[ 1 ] );
                builder.end_object();
            },
            [&]< class Ty >( const Math::cVector< 3, Ty >& _vector )
            {
                builder.start_object();
                builder.append_key_value( "x", _vector[ 0 ] );
                builder.append_comma();
                builder.append_key_value( "y", _vector[ 1 ] );
                builder.append_comma();
                builder.append_key_value( "z", _vector[ 2 ] );
                builder.end_object();
            },
            [&]< class Ty >( const Math::cVector< 4, Ty >& _vector )
            {
                builder.start_object();
                builder.append_key_value( "x", _vector[ 0 ] );
                builder.append_comma();
                builder.append_key_value( "y", _vector[ 1 ] );
                builder.append_comma();
                builder.append_key_value( "z", _vector[ 2 ] );
                builder.append_comma();
                builder.append_key_value( "w", _vector[ 3 ] );
                builder.end_object();
            },
            [&]( const cWeak_Ptr< cAsset_Meta >& _meta )
            {
                builder.start_object();
                builder.append_key_value( "asset_uuid", _meta.is_valid() ? _meta->GetUUID().ToString() : "" );
                builder.append_comma();
                builder.append_key_value( "asset_path", _meta.is_valid() ? _meta->GetPath().string() : "" );
                builder.end_object();
            }
        }( _value );
    }
} // ::

void cSerializedObject::create_json_array( json_builder_t& _builder )
{
    auto& builder = _builder;
    
    builder.start_array();

    if( m_element_count_ > 0 )
    {
        std::visit( [&]< class V >( V& _array ){
            if constexpr( !std::is_same_v< V, std::monostate > )
            {
                for( size_t i = 0; i < m_element_count_; i++ )
                {
                    handle_value( _builder, _array[ i ] );
                    if( i != m_element_count_ - 1 )
                        builder.append_comma();
                }
            }
        }, m_element_data_ );
    }

    builder.end_array();
}

void cSerializedObject::handle_info( json_builder_t& _builder, const sValueInfo& _info )
{
    auto& info    = _info;
    auto& element = m_values_[ info.value_index ];

    sVisitor visitor{
        [&]( auto& _value )
        {
            handle_value( _builder, _value );
        }
    };

    std::visit( visitor, element );
}

void cSerializedObject::handle_json_element( const simdjson::ondemand::value& _element, const std::string_view _key, Serialization::cResources* _resources )
{
    m_values_.emplace_back( handle_json_element_( _element, _key, _resources ) );
}

void cSerializedObject::_writeData( const cStringID& _name, value_t&& _value )
{
    const auto json_safe_name = MakeJsonSafeName( _name.view() );

    auto index = m_info_.size();
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

auto cSerializedObject::get_value_at_offset( type_info_t _type, size_t _offset ) -> std::optional< value_t >
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

bool cSerializedObject::has_completed_json() const
{
    // The JSON builder will never have any data unless it's completed.
    return m_json_builder_.size() > 0;
}
