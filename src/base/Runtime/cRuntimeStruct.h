/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <Macros/for_each.h>
#include <Macros/manipulation.h>

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

#define REGISTER_STRUCT_TYPE( Type, Types, Name ) \
template<> struct qw::get_type_id< Type >{ \
constexpr static qw::type_hash kId = qw::args_hash< Types >::kHash; \
constexpr static char kName[] = Name; };

#define BUILD_TYPE_INFO( Name, ... ) REGISTER_STRUCT_TYPE( Name, GET_MEMBER_TYPES( __VA_ARGS__ ), #Name )

#define MAKE_STRUCT( Name, ... ) \
BUILD_STRUCT_BODY( Name, __VA_ARGS__ ) \
BUILD_TYPE_INFO( Name, __VA_ARGS__ )
 

MAKE_STRUCT( TestStruct,
	M( int32_t, Count ),
	M( int32_t, Count2 ),
	M( int32_t, Count3 ),
)
struct TestStruct2 { int32_t Count ; int32_t Count2 ; int32_t Count3 ; };
template<> struct qw::get_type_id< TestStruct2 >{
constexpr static type_hash kId = args_hash< int32_t , int32_t , int32_t >::kHash;
constexpr static char kName[] = "TestStruct2"; };
