/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <algorithm>

#include "Misc/Hashing.h"
#include "Misc/string_manipulation.h"

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

    template< class Ty >
    struct get_type_id
    {
        static_assert( false, "Invalid Type" );
        constexpr static qw::type_hash kId     = kInvalid_Id;
        constexpr static char          kName[] = "Invalid";
    };

    template< class Ty >
    constexpr uint64_t get_type_hash( void ) // Make some version already applying the prime_64_const
    {
            typedef qw::get_type_id< Ty > type_container_t;
            static_assert( type_container_t::kId != qw::kInvalid_Id, "The type specified isn't registered as a valid Type." );

            return type_container_t::kId.getHash();
    } // get_type_hash

    template< class... Args >
    struct args_hash
    {
    };
    template< class Ty, class... Args >
    struct args_hash< Ty, Args... >
    {
        typedef args_hash< Args... > previous_t;
        constexpr static size_t   kCount = 1 + previous_t::kCount;
        constexpr static uint64_t kHash  = ( previous_t::kHash ^ get_type_hash< Ty >() ) * Hashing::prime_64_const;
        constexpr static array    kTypes = array{ get_type_id< Ty >::kId } + previous_t::kTypes;
    };
    template< class Ty >
    struct args_hash< Ty >
    {
        constexpr static size_t   kCount = 1;
        constexpr static uint64_t kHash  = get_type_hash< Ty >() * Hashing::prime_64_const;
        constexpr static array    kTypes{ get_type_id< Ty >::kId };
    };
} // qw::

#define REGISTER_TYPE_DIRECT( Type, HashValue, Name ) \
template<> struct qw::get_type_id< Type >{ \
constexpr static qw::type_hash kId{ HashValue }; \
constexpr static char kName[] = Name; };

#define REGISTER_T_TYPE( Type, Name ) REGISTER_TYPE_DIRECT( Type ## _t, #Type, Name )
#define REGISTER_TYPE( Type, Name ) REGISTER_TYPE_DIRECT( Type, #Type, Name )
#define REGISTER_TYPE_AS( Type, Name, HashVal ) REGISTER_TYPE_DIRECT( Type, HashVal, Name )

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

