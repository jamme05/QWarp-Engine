

#pragma once

#include <sk/Math/Vector2.h>
#include <sk/Math/Vector3.h>
#include <sk/Math/Vector4.h>
#include <sk/Misc/Smart_Ptrs.h>
#include <sk/Misc/StringID.h>
#include <sk/Misc/UUID.h>
#include <sk/Misc/Visitor.h>
#include <sk/Reflection/RuntimeClass.h>

#include <simdjson.h>

#include <variant>


namespace sk
{
    class cAsset_Meta;

    namespace Serialization
    {
        class cResources
        {
        public:
            using builder_t = simdjson::builder::string_builder;

            cResources() = default;
            explicit cResources( simdjson::ondemand::object _object );

            using type_map_t  = std::unordered_map< type_hash,     type_info_t >;
            using asset_map_t = std::unordered_map< hash< cUUID >, cWeak_Ptr< cAsset_Meta > >;

            void StoreAsset( const cWeak_Ptr< cAsset_Meta >& _asset_meta );
            void StoreType ( const type_info_t& _type );
            auto GetAsset  ( const cUUID& _id ) -> cWeak_Ptr< cAsset_Meta >;
            auto GetType   ( uint64_t _id ) -> type_info_t;

            bool IsEmpty() const;

            auto CreateJSON() -> std::string_view;

            builder_t   builder;

            type_map_t  types;
            asset_map_t assets;
        };
    } // sk::Serialization::

    SK_CLASS( SerializedObject )
    {
        SK_CLASS_BODY( SerializedObject )
        friend class Serializable;
    public:
        template< reflected Ty >
        explicit cSerializedObject( Ty* _instance )
        {
            m_serialized_type_ = kTypeInfo< Ty >;
            BeginWrite( _instance );
        }

        explicit cSerializedObject( type_info_t _serialized_type, size_t _element_count = 0 );
        // This is so we can ignore the full on class reflection.

        struct sValueInfo
        {
            enum eFlags : uint8_t
            {
                kNone = 0,
                
                kRealMember = 0x01,
            };
            cStringID   name;
            cStringID   json_safe_name;
            // Binary offset, for member variable lookups.
            size_t      offset;
            uint32_t    value_index;
            uint32_t    flags;
            type_info_t type;
        };
        
        cSerializedObject();
        explicit cSerializedObject( simdjson::ondemand::object _object, Serialization::cResources* _resources = nullptr );
        explicit cSerializedObject( simdjson::ondemand::array  _array, size_t _length = std::numeric_limits< size_t >::max(),
            const std::string_view& _key = {}, Serialization::cResources* _resources = nullptr );
        cSerializedObject( const cSerializedObject& _other );
        cSerializedObject( cSerializedObject&& _other ) noexcept;
        ~cSerializedObject() override;

        cSerializedObject& operator=( const cSerializedObject& _other );
        cSerializedObject& operator=( cSerializedObject&& _other ) noexcept;
        
        template< reflected Ty >
        static auto CreateArray( Ty* _data, size_t _size )  -> cSerializedObject;

        template< reflected Ty >
        static auto ConsumeArray( Ty* _data, size_t _size )  -> cSerializedObject;

        template< class Ty, class Value >
        struct sMemberVariable
        {
            using class_type = Ty;
            using value_type = Value;
            
            consteval sMemberVariable( Value Ty::* _ptr )
            : ptr( _ptr )
            {}
            Value Ty::* ptr;
        };

        using obj_t    = cSerializedObject;
        using meta_ptr = cWeak_Ptr< cAsset_Meta >;
        using value_t = std::variant< std::monostate, obj_t,  meta_ptr,  bool,  int64_t,  cVector2i64,  cVector3i64,  cVector4i64,  uint64_t,  cVector2u64,  cVector3u64,  cVector4u64,  double,  cVector2d,  cVector3d,  cVector4d,  std::string  >;
        using array_t = std::variant< std::monostate, obj_t*, meta_ptr*, bool*, int64_t*, cVector2i64*, cVector3i64*, cVector4i64*, uint64_t*, cVector2u64*, cVector3u64*, cVector4u64*, double*, cVector2d*, cVector3d*, cVector4d*, std::string* >;
        using json_builder_t = simdjson::builder::string_builder;

        [[ nodiscard ]] bool IsArray() const;
        
        void Reset();
        
        // Creators
        [[ nodiscard ]]
        auto CreateJSON() -> std::string_view;
        [[ nodiscard ]]
        auto CreateBinary() -> std::span< std::byte >;
        
        void ClearCache();

        [[ nodiscard ]] auto GetType() const -> type_info_t;
        [[ nodiscard ]] auto GetRuntimeClass() const -> class_info_t*;

        auto GetBase( type_info_t _type ) -> std::optional< std::reference_wrapper< cSerializedObject > >;
        template< reflected Ty >
        auto GetBase() -> std::optional< std::reference_wrapper< cSerializedObject > >;

        auto ConstructClass() -> iClass*;
        auto ConstructSharedClass() -> cShared_ptr< iClass >;
        template< sk_class Ty >
        auto ConstructClassAs() -> Ty*;
        template< sk_class Ty >
        auto ConstructSharedClassAs() -> cShared_ptr< Ty >;

        // Read
        void BeginRead( iClass* _this = nullptr );
        auto ReadValueRaw( const cStringID& _name ) -> std::optional< std::reference_wrapper< value_t > >;

        template< class Ty >
        bool TryReadValue( const cStringID& _name, Ty& _out )
        {
            const auto data = ReadValueRaw( _name );
            static_assert( !std::is_same_v< Ty, cSerializedObject >, "Error: Don't use a copy of a SerializedObject use a pointer instead." );
            if constexpr( std::is_same_v< std::remove_const_t< Ty >, cSerializedObject* > )
            {
                if( data.has_value() )
                {
                    if( const auto res = std::get_if< cSerializedObject >( &data.value().get() ) )
                    {
                        _out = res;
                        return true;
                    }
                }
            }
            else if constexpr( std::is_same_v< Ty, std::string > || std::is_same_v< Ty, std::string_view > )
            {
                if( data.has_value() )
                {
                    if( const auto res = std::get_if< std::string >( &data.value().get() ) )
                    {
                        _out = Ty{ *res };
                        return true;
                    }
                }
            }
            else
            {
                if( data.has_value() )
                {
                    return std::visit( [&]< class V >( V& _value ) -> bool{
                        if constexpr( std::is_same_v< Ty, V > && std::is_same_v< V, cSerializedObject > )
                            return false;
                        else if constexpr( std::is_same_v< V, Ty > )
                        {
                            _out = _value;
                            return true;
                        }
                        else if constexpr( std::is_convertible_v< V, Ty > )
                        {
                            _out = static_cast< V >( _value );
                            return true;
                        }
                        else if constexpr( std::is_integral_v< V > && std::is_enum_v< Ty > )
                        {
                            _out = static_cast< Ty >( _value );
                            return true;
                        }
                        else
                            return false;
                    }, data.value().get() );
                }
            }
            return false;
        }

        template< class Ty >
        auto ReadValue( const cStringID& _name )
        {
            if( Ty result; TryReadValue( _name, result ) )
                return result;
            SK_FATAL( "Error: No value exists at name {}", _name.view() );
        }

        template< class Ty >
        auto ReadValueOr( const cStringID& _name, Ty&& _fallback )
        {
            if( Ty result; TryReadValue( _name, result ) )
                return result;
            return std::forward< Ty >( _fallback );
        }

        template< class Ty >
        auto ReadValueOr( const cStringID& _name, const Ty& _fallback )
        {
            if( Ty result; TryReadValue( _name, result ) )
                return result;
            return _fallback;
        }

        auto GetArraySize() const -> size_t;
        template< class Ty >
        auto GetArray() -> std::span< Ty >;
        void EndRead ();

        template< sMemberVariable Target, reflected Value = decltype( Target )::value_type >
        void ReadIntoThis();
        
        // Write
        auto BeginWrite( iClass* _this = nullptr, bool _reset = false ) -> cSerializedObject&;
        auto AddBase( cSerializedObject&& _base_info ) -> cSerializedObject&;
        auto WriteValue( const cStringID& _name, auto&& _value ) -> cSerializedObject&;
        auto EndWrite() -> cSerializedObject&&;

        static std::string MakeJsonSafeName( const std::string_view& _name );
        
        void createJsonInternal( json_builder_t& _builder, std::string* _type, Serialization::cResources* _resources = nullptr );
    private:
        void _createJsonObject( json_builder_t& _builder );
        void _createJsonArray( json_builder_t& _builder, std::string& _type );
        void _handleInfo( json_builder_t& _builder, const sValueInfo& _info, const std::string& _key );
        void _handleJsonElement(const simdjson::ondemand::value& _element, std::string_view _key );

        void _writeData( const cStringID& _name, value_t&& _value );
        
        auto _getValueAtOffset( type_info_t _type, size_t _offset ) -> std::optional< value_t >;
        bool _hasCompletedJson() const;
        
        using info_vec_t  = std::vector< sValueInfo >;
        using value_vec_t = std::vector< value_t >;
        using obj_vec_t   = std::vector< obj_t >;
        using buffer_t    = std::vector< std::byte >;

        Serialization::cResources* m_resources_ = nullptr;

        // Info
        type_info_t m_serialized_type_;
        // Temporary this
        iClass*     m_this_ = nullptr;
        // The offset to where the class reflection data starts.
        size_t      m_raw_offset_ = 0;
        
        // TODO: Create a separate object for handling arrays.
        // Array info:
        size_t  m_element_count_ = 0; // Zero if the object isn't an array.
        array_t m_element_data_;
        size_t  m_offset_ = 0;
        
        // Cache
        json_builder_t m_json_builder_;
        buffer_t       m_binary_cache_;

        // Base classes
        obj_vec_t m_bases_;

        // Held values
        info_vec_t  m_info_;
        value_vec_t m_values_;
    };

    template< reflected Ty >
    auto cSerializedObject::CreateArray( Ty* _data, size_t _size ) -> cSerializedObject
    {
        Ty* data_ptr = nullptr;
        if( _size == 0 )
            _size = std::numeric_limits< size_t >::max();
        else
        {
            data_ptr = SK_NEW( Ty, _size );
            std::copy_n( _data, _size, data_ptr );
        }
        cSerializedObject object{ kTypeInfo< Ty >, _size };
        object.m_element_data_ = data_ptr;
        
        return object;
    }

    template< reflected Ty >
    auto cSerializedObject::ConsumeArray( Ty* _data, size_t _size ) -> cSerializedObject
    {
        Ty* data_ptr = nullptr;
        if( _size == 0 )
            _size = 0;
        else
        {
            data_ptr = SK_NEW( Ty, _size );
            std::move( _data, _data + _size, data_ptr );
        }
        cSerializedObject object{ kTypeInfo< Ty >, _size };
        object.m_element_data_ = data_ptr;

        return object;
    }

    template< reflected Ty >
    auto cSerializedObject::GetBase() -> std::optional< std::reference_wrapper< cSerializedObject > >
    {
        return GetBase( kTypeInfo< Ty > );
    }

    template< sk_class Ty >
    auto cSerializedObject::ConstructClassAs() -> Ty*
    {
        return static_cast< Ty* >( ConstructClass() );
    }

    template< sk_class Ty >
    auto cSerializedObject::ConstructSharedClassAs() -> cShared_ptr< Ty >
    {
        return ConstructSharedClass().Cast< Ty >();
    }

    template< class Ty >
    auto cSerializedObject::GetArray() -> std::span< Ty >
    {
        // TODO: Ensure that the type is correct.
        return std::span< Ty >{ std::get< Ty* >( m_element_data_ ), m_element_count_ };
    }

    template< cSerializedObject::sMemberVariable Target, reflected Value >
    void cSerializedObject::ReadIntoThis()
    {
        using class_type = decltype( Target )::class_type;
        
        auto& value = _getValueAtOffset( kTypeInfo< Value >, offset_of< Target >() );
        
        SK_ERR_IFN( value.has_value(), "Error: No value found." )
        
        static_cast< class_type* >( m_this_ )->*Target.ptr = value;
    }

    auto cSerializedObject::WriteValue( const cStringID& _name, auto&& _value ) -> cSerializedObject&
    {
        using value_type = std::remove_cvref_t< decltype( _value ) >;

        if constexpr( std::is_enum_v< value_type > )
        {
            if constexpr( std::is_signed_v< std::underlying_type_t< value_type > > )
                _writeData( _name, value_t{ static_cast< int64_t >( std::forward< decltype( _value ) >( _value ) ) } );
            else
                _writeData( _name, value_t{ static_cast< uint64_t >( std::forward< decltype( _value ) >( _value ) ) } );
        }
        else if constexpr( std::is_integral_v< value_type > && !std::is_same_v< value_type, bool > )
        {
            if constexpr( std::is_signed_v< value_type > )
                _writeData( _name, value_t{ static_cast< int64_t >( std::forward< decltype( _value ) >( _value ) ) } );
            else
                _writeData( _name, value_t{ static_cast< uint64_t >( std::forward< decltype( _value ) >( _value ) ) } );
        }
        else
            _writeData( _name, value_t{ std::forward< decltype( _value ) >( _value ) } );
        return *this;
    }
} // sk::

constexpr bool whatthefuck = std::is_convertible_v< sk::cSerializedObject, sk::iClass >;

SK_DECLARE_CLASS( sk::SerializedObject )
