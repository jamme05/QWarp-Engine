/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <algorithm>

#include "Misc/Hashing.h"

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
    
    constexpr static qw::type_hash kInvalid_Id = 0;

    template< class Ty >
    struct get_type_id
    {
        static_assert( false, "Invalid Type" );
        constexpr static qw::type_hash kId     = kInvalid_Id;
        constexpr static char          kName[] = "Invalid";
    };
} // qw::

#define REGISTER_TYPE_DIRECT( Type, HashValue, Name ) \
template<> struct qw::get_type_id< Type >{ \
constexpr static qw::type_hash kId{ HashValue }; \
constexpr static char kName[] = Name; };

#define REGISTER_T_TYPE( Type, Name ) REGISTER_TYPE_DIRECT( Type ## _t, #Type, Name )
#define REGISTER_TYPE( Type, Name ) REGISTER_TYPE_DIRECT( Type, #Type, Name )
#define REGISTER_TYPE_AS( Type, Name, HashVal ) REGISTER_TYPE_DIRECT( Type, HashVal, Name )
