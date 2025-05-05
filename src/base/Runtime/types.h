/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Misc/Hashing.h"
#include "Misc/string_manipulation.h"
#include "Containers/const_map.h"

#include <Macros/manipulation.h>

#include <Containers/map.h>

#include "Macros/enum_builder.h"

namespace qw
{
    template<>
    class hash< void > : public Hashing::iHashed
    {
    public:
        constexpr hash( void )
        : iHashed( 0 )
        {} // Invalid type
        constexpr hash( const str_hash& _str_hash )
        : iHashed( _str_hash.getHash() )
        {
        } // type_hash

        constexpr hash( const uint64_t _raw_value )
        : iHashed( _raw_value )
        {
        } // type_hash

        HASH_REQUIREMENTS( hash )
    };

    typedef hash< void > type_hash;
    // TODO: Create a file for just the type hash.

    constexpr static type_hash kInvalid_Id = 0;

    struct sType_Info;
    extern const unordered_map< type_hash, const sType_Info* > type_map;
    typedef std::pair< type_hash, const sType_Info* > type_pair_t;

    struct sType_Info
    {
        MAKE_ENUM( ENUMCLASS( eType ),
            E( kStandard, "Standard" ),
            E( kEnum, "Enum" ),
            E( kArray, "Array" ),
            E( kStruct, "Struct" ),
            E( kClass, "Class" )
        );

        eType       type;
        type_hash   hash;
        size_t      size;
        const char* name;
        const char* raw_name;
        bool        is_ptr = false;
        bool        is_ref = false;
        constexpr sType_Info as_ptr( void ) const
        {
            return { .hash = hash, .size = size, .name = name, .raw_name = raw_name, .is_ptr = true };
        } // as_ptr
        constexpr sType_Info as_ref( void ) const
        {
            return { .hash = hash, .size = size, .name = name, .raw_name = raw_name, .is_ref = true };
        } // as_ref
        constexpr type_pair_t pair( void ) const
        {
            return { hash, this };
        } // pair
    };

    // TODO: Move individual types to their own spaces.
    namespace runtime_struct
    {
        struct sMemberInfo
        {
            type_hash   type;
            str_hash    name_hash; // Original name shouldn't be required to be accessed?
            const char* display_name;
            size_t      size;
            size_t      offset;
            [[nodiscard]] const sType_Info* get_type( void ) const;
        };

        inline const sType_Info* sMemberInfo::get_type( void ) const
        {
            if( const auto it = type_map.find( type ); it != type_map.end() )
                return it->second;
            return nullptr;
        } // get_type
    } // runtime_struct::

    struct sStruct_Type_Info : sType_Info
    {
        map_ref< str_hash, runtime_struct::sMemberInfo, std::less< str_hash > > members;
    };

    struct sEnum_Type_Info : sType_Info
    {
        
    };

    template< class Ty >
    struct get_type_info
    {
        // static_assert( false, "Invalid Type" );
        constexpr static sType_Info kInfo = {
            .hash = kInvalid_Id,
            .size = 0,
            .name = "Invalid",
            .raw_name = "invalid"
        };
        constexpr static bool kValid  = false;
    };
    template< class Ty >
    requires get_type_info< Ty >::kValid
    struct get_type_info< Ty* > : get_type_info< Ty >
    {
        typedef get_type_info< Ty > raw_t;
        constexpr static sType_Info kInfo = raw_t::kInfo.as_ptr();
    };
    template< class Ty >
    requires get_type_info< Ty >::kValid
    struct get_type_info< Ty& > : get_type_info< Ty >
    {
        typedef get_type_info< Ty > raw_t;
        constexpr static sType_Info kInfo = raw_t::kInfo.as_ref();
    };
    template< class Ty >
    requires get_type_info< Ty >::kValid
    struct get_type_hash
    {
        constexpr static type_hash value = get_type_info< Ty >::kInfo.hash;
    };
    template< class Ty >
    requires get_type_info< Ty >::kValid
    constexpr inline type_hash get_type_hash_v = get_type_hash< Ty >::value;

    template< class Ty >
    constexpr static auto& kTypeId = get_type_info< Ty >::kInfo;

    template< class Ty >
    constexpr uint64_t get_type_value( void ) // Make some version already applying the prime_64_const
    {
            typedef get_type_info< Ty > type_container_t;
            static_assert( type_container_t::kValid, "The type specified isn't registered as a valid Type." );

            return type_container_t::kInfo.hash.getHash();
    } // get_type_hash

    enum eArgumentError : uint8_t
    {
        kValid = 0, // The argument types are valid.
        kVoidNotAllowed = 1, // Void isn't allowed in this scenario.
        kVoidNotOnly   = 2,  // Void isn't the only type in this scenario.
    };
    template< size_t Size >
    constexpr uint8_t validate_args( const array< type_hash, Size >& _array, const bool _allow_void = true );

    template< class Ty >
    struct valid_type
    {
        constexpr static bool value = get_type_info< Ty >::kValid;
    };
    template< class Ty >
    constexpr inline bool kValidType = valid_type< Ty >::value;

    template< class... Types >
    constexpr inline bool kValidTypes = std::conjunction_v< valid_type< Types >... >;
    template<>
    constexpr inline bool kValidTypes<> = false;
    template< bool AllowVoid, class... Types >
    struct types_hash
    {
    };
    template< bool AllowVoid, class Ty, class... Types >
    requires kValidTypes< Ty, Types... >
    struct types_hash< AllowVoid, Ty, Types... >
    {
        typedef types_hash< AllowVoid, Types... > previous_t;
        constexpr static size_t   kCount = 1 + previous_t::kCount;
        constexpr static uint64_t kHash  = ( previous_t::kHash ^ get_type_value< Ty >() ) * Hashing::prime_64_const;
        constexpr static auto     kTypes = array{ get_type_info< Ty >::kInfo.hash } + previous_t::kTypes;
        constexpr static auto     kError = validate_args( kTypes, AllowVoid );
        // TODO: Some way to get the types as a string for debugging.
        static_assert( !( kError & kVoidNotAllowed ), "Void found but was not allowed in this scenario." );
        static_assert( !( kError & kVoidNotOnly ),    "Void isn't the only argument type in this scenario." );
    };
    template< bool AllowVoid >
    struct types_hash< AllowVoid >
    {
        constexpr static size_t   kCount = 0;
        constexpr static uint64_t kHash  = Hashing::prime_64_const;
        constexpr static auto     kTypes = array< type_hash, 0 >{};
    };
    template< class... Types >
    using args_hash = types_hash< true, Types... >;
    template< class... Types >
    using struct_hash = types_hash< false, Types... >;
} // qw::

// Pre-declare void type.
template<> struct qw::get_type_info< void >
{
    constexpr static sType_Info kInfo   = { .hash = { "Void" }, .size = 0, .name = "Void", .raw_name = "void" };
    constexpr static bool      kValid = true;
}; // Void my beloved

template< size_t Size >
constexpr uint8_t qw::validate_args( const array< type_hash, Size >& _array, const bool _allow_void )
{
    // TODO: Decide if void* should be allowed in the reflection system.
    if constexpr( Size == 1 )
    {
        // If not void it's always going to be valid.
        if( _array[ 0 ] != get_type_hash_v< void > )
            return kValid;
        // Otherwise the bool decides the fate.
        return _allow_void ? kValid : kVoidNotAllowed; // -1 = Type void found but was not allowed.
    }
    else
    {
        // Validate so there's only a single void type.
        for( size_t i = 0; i < Size; ++i )
        {
            // in case it has a void type, it will always be an error.
            if( _array[ i ] == get_type_hash_v< void > )
            {
                if( !_allow_void )
                    return kVoidNotAllowed | kVoidNotOnly; // -1 = Type void found but was not allowed
                return kVoidNotOnly; // -2 = Type void is only allowed if it's the only type.
            }
        }
            
    }
    return kValid; // Nothing wrong.
}

// Counter from: https://stackoverflow.com/a/74453799
template<auto Id>
struct counter {
    using tag = counter;

    struct generator {
        friend consteval auto is_defined(tag)
        { return true; }
    };
    friend consteval auto is_defined(tag);

    template<typename Tag = tag, auto = is_defined(Tag{})>
    static consteval auto exists(auto)
    { return true; }

    static consteval auto exists(...)
    { return generator(), false; }
};

template<auto Id = int{}, typename = decltype([]{})>
consteval auto unique_id() {
    if constexpr (not counter<Id>::exists(Id)) return Id;
    else return unique_id<Id + 1>();
}

namespace qw::registry
{
    template< int64_t Iteration >
    struct type_registry
    {
        constexpr static array< type_pair_t, 0 > registered = {};
        constexpr static bool                    valid      = false;
    };
} // qw::

#if defined( IN_EDITOR )
#define REGISTER_TYPE_INTERNAL( Type ) COMMENT( Placeholder for type registry. )
#else // IN_EDITOR
#define REGISTER_TYPE_INTERNAL_0( Type, IdLocation ) \
constexpr static auto IdLocation = unique_id(); \
template<> struct qw::registry::type_registry< IdLocation >{ \
    typedef type_registry< IdLocation - 1 > previous_t; \
    constexpr static auto   Iteration  = IdLocation; \
    constexpr static array  registered = previous_t::registered + array{ get_type_info< Type >::kInfo.pair() }; \
    constexpr static bool valid = true; \
};
#define REGISTER_TYPE_INTERNAL( Type ) REGISTER_TYPE_INTERNAL_0( Type, CONCAT( type_registry_, __COUNTER__ ) )
#endif // !IN_EDITOR

// Finally register void as everything exists.
REGISTER_TYPE_INTERNAL( void )

#define MAKE_TYPE_INFO_DIRECT( Type, Name, HashMacro, ... ) \
template<> struct qw::get_type_info< Type >{ \
constexpr static sType_Info kInfo   = { .type = sType_Info::eType::kStandard, .hash HashMacro( __VA_ARGS__ ) , .size = sizeof( Type ), .name = Name, .raw_name = #Type }; \
constexpr static bool      kValid = true; \
};

#define MAKE_DEFAULT_HASH( HashName, ... ) { HashName }
#define REGISTER_DEFAULT_TYPE( Type, InternalName, Name ) \
    MAKE_TYPE_INFO_DIRECT( Type, Name, MAKE_DEFAULT_HASH, Name ) \
    REGISTER_TYPE_INTERNAL( Type )

#define REGISTER_T_TYPE( Type, Name ) REGISTER_DEFAULT_TYPE( Type ## _t, #Type, Name )
#define REGISTER_TYPE( Type, Name ) REGISTER_DEFAULT_TYPE( Type, #Type, Name )

REGISTER_TYPE  ( bool, "Boolean" )
REGISTER_T_TYPE( int8, "Char" )
REGISTER_T_TYPE( uint8, "Byte" )
REGISTER_T_TYPE( int16, "Short" )
REGISTER_T_TYPE( uint16, "Unsigned Short" )
REGISTER_T_TYPE( int32, "Integer" )
REGISTER_T_TYPE( uint32, "Unsigned Integer" )
REGISTER_T_TYPE( int64, "Long" )
REGISTER_T_TYPE( uint64, "Unsigned Long" )

REGISTER_TYPE( float, "Float" )
REGISTER_TYPE( double, "Double" )

