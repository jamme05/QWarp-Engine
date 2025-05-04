/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <Macros/for_each.h>
#include <Macros/manipulation.h>
#include <Runtime/types.h>

#define GET_MEMBER_M( Type, Name, ... ) Type Name ;
#define GET_TYPE_M( Type, Name, ... ) Type
#define GET_NAME_M( Type, Name, ... ) Name

#define BUILD_STRUCT_BEGIN( Name ) struct Name {
#define BUILD_STRUCT_MEMBER( Arg ) GET_MEMBER_ ## Arg
#define BUILD_STRUCT_TYPE( Arg, ... ) GET_TYPE_ ## Arg __VA_OPT__(,)

#define GET_MEMBER_TYPES( ... ) FOR_EACH_FORWARD( BUILD_STRUCT_TYPE, __VA_ARGS__ )

#define BUILD_STRUCT_MEMBERS( ... ) FOR_EACH( BUILD_STRUCT_MEMBER, __VA_ARGS__ )
#define BUILD_STRUCT_BODY( Name, ... ) \
BUILD_STRUCT_BEGIN( Name ) \
BUILD_STRUCT_MEMBERS( __VA_ARGS__ ) \
};

#define GET_ARGS_HASH( ... ) qw::struct_hash< __VA_ARGS__ >::kHash

#define MAKE_STRUCT_TYPE_INFO( Type, MembersHash, Members ) \
template<> struct qw::get_type_info< Type >{ \
	constexpr static sStruct_Type_Info kInfo = { .hash HashMacro( __VA_ARGS__ ) , .size = sizeof( Type ), .name = Name, .raw_name = #Type }; \
	constexpr static bool      kValid = true; \
	};


#define REGISTER_STRUCT( Type, Types, Name ) \
	MAKE_TYPE_INFO_DIRECT( Type, Name, = GET_ARGS_HASH, Types ) \
	REGISTER_TYPE_INTERNAL( Type )

#define BUILD_TYPE_INFO( Name, ... ) REGISTER_STRUCT( Name, GET_MEMBER_TYPES( __VA_ARGS__ ), #Name )

#define MAKE_STRUCT( Name, ... ) \
BUILD_STRUCT_BODY( Name, __VA_ARGS__ ) \
BUILD_TYPE_INFO( Name, __VA_ARGS__ )

MAKE_STRUCT( TestStruct,
	M( int32_t, Count ),
	M( int32_t, Count2 ),
	M( int32_t, Count3 ),
	M( int8_t, Count4 ),
	M( int32_t, Count5 ),
)

//MAKE_TYPE_INFO_DIRECT( Type, Name, = GET_ARGS_HASH, Types )