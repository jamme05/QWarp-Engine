/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <algorithm>

#include "Misc/Hashing.h"
#include "Misc/string_manipulation.h"

#include <Containers/map.h>

namespace qw
{
    template<>
    class hash< void > : public Hashing::iHashed
    {
    public:
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

    constexpr static type_hash kInvalid_Id = 0;

    struct type_info
    {
        type_hash   hash;
        size_t      size;
        const char* name;
    };

    // TODO: Add actual type instead of void*.
    static unordered_map< type_hash, void* > type_map = {};

    template< class Ty >
    struct get_type_id
    {
        static_assert( false, "Invalid Type" );
        constexpr static qw::type_hash kId     = kInvalid_Id;
        constexpr static char          kName[] = "Invalid";
        constexpr static type_info     kInfo   = { .hash = kId, .size = 0, .name = kName };
    };

    template< class Ty >
    constexpr static auto& kTypeId = get_type_id< Ty >::kInfo;

    template< class Ty >
    constexpr uint64_t get_type_hash( void ) // Make some version already applying the prime_64_const
    {
            typedef qw::get_type_id< Ty > type_container_t;
            static_assert( type_container_t::kId != qw::kInvalid_Id, "The type specified isn't registered as a valid Type." );

            return type_container_t::kId.getHash();
    } // get_type_hash

    enum eArgumentError : uint8_t
    {
        kValid = 0, // The argument types are valid.
        kVoidNotAllowed = 1, // Void isn't allowed in this scenario.
        kVoidNotOnly   = 2,  // Void isn't the only type in this scenario.
    };
    template< size_t Size >
    constexpr uint8_t validate_args( const array< type_hash, Size > _array, const bool _allow_void = true );

    template< bool AllowVoid, class... Args >
    struct args_hash
    {
    };
    template< bool AllowVoid, class Ty, class... Args >
    struct args_hash< AllowVoid, Ty, Args... >
    {
        typedef args_hash< AllowVoid, Args... > previous_t;
        constexpr static size_t   kCount = 1 + previous_t::kCount;
        constexpr static uint64_t kHash  = ( previous_t::kHash ^ get_type_hash< Ty >() ) * Hashing::prime_64_const;
        constexpr static array    kTypes = array{ get_type_id< Ty >::kId } + previous_t::kTypes;
        constexpr static auto     kError = validate_args( kTypes, AllowVoid );
        static_assert( !( kError & kVoidNotAllowed ), "Void found but was not allowed in this scenario." );
        static_assert( !( kError & kVoidNotOnly ),    "Void isn't the only argument type in this scenario." );
    };
    template< bool AllowVoid >
    struct args_hash< AllowVoid >
    {
        constexpr static size_t   kCount = 0;
        constexpr static uint64_t kHash  = Hashing::prime_64_const;
        constexpr static array< type_hash, 0 > kTypes{ };
    };
} // qw::

#define REGISTER_TYPE_DIRECT( Type, Name, HashMacro, ... ) \
template<> struct qw::get_type_id< Type >{ \
constexpr static qw::type_hash kId HashMacro( __VA_ARGS__ ) ; \
constexpr static char kName[] = Name; };

#define MAKE_DEFAULT_HASH( HashName, ... ) { HashName }
#define REGISTER_DEFAULT_TYPE( Type, InternalName, Name ) REGISTER_TYPE_DIRECT( Type, Name, MAKE_DEFAULT_HASH, Name )

#define REGISTER_T_TYPE( Type, Name ) REGISTER_DEFAULT_TYPE( Type ## _t, #Type, Name )
#define REGISTER_TYPE( Type, Name ) REGISTER_DEFAULT_TYPE( Type, #Type, Name )
#define REGISTER_TYPE_AS( Type, Name, HashVal ) REGISTER_DEFAULT_TYPE( Type, HashVal, Name )

REGISTER_TYPE_AS( void, "Void", 1 ) // Void my beloved
REGISTER_TYPE( bool, "Boolean" )
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

// TODO: Make a runtime type manager.

template< size_t Size >
inline constexpr uint8_t qw::validate_args( const array< type_hash, Size > _array, const bool _allow_void )
{
    if constexpr( Size == 1 )
    {
        // If not void it's always going to be valid.
        if( _array[ 0 ] != get_type_id< void >::kId )
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
            if( _array[ i ] == get_type_id< void >::kId )
            {
                if( !_allow_void )
                    return kVoidNotAllowed | kVoidNotOnly; // -1 = Type void found but was not allowed
                return kVoidNotOnly; // -2 = Type void is only allowed if it's the only type.
            }
        }
            
    }
    return kValid; // Nothing wrong.
}
