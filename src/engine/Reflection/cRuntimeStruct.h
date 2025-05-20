/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <Macros/for_each.h>
#include <Macros/manipulation.h>
#include <Runtime/types.h>

#define UNWRAP_M( ... ) __VA_ARGS__
#define GET_MEMBER_M( Type, Name, ... ) Type Name ;
#define GET_TYPE_M( Type, Name, ... ) Type
#define GET_NAME_M( Type, Name, ... ) Name

#define BUILD_STRUCT_BEGIN( Name ) struct Name {
#define BUILD_STRUCT_MEMBER( Arg ) GET_MEMBER_ ## Arg
#define BUILD_STRUCT_TYPE( Arg, ... ) GET_TYPE_ ## Arg __VA_OPT__(,)

#define GET_MEMBER_TYPES( ... ) FOR_EACH_FORWARD( BUILD_STRUCT_TYPE, __VA_ARGS__ )

#define BUILD_STRUCT_MEMBERS( ... ) FOR_EACH( BUILD_STRUCT_MEMBER, __VA_ARGS__ )

#define GET_MEMBER_INFO( Struct, Type, Name, ... ) runtime_struct::sMemberInfo{ qw::kTypeId< Type >.hash, #Name, #Name, sizeof( Type ), offsetof( Struct, Name ) }
#define GET_MEMBER_REFLECTION_0( Struct, Type, Name, ... ) std::pair{ str_hash{ #Name }, GET_MEMBER_INFO( Struct, Type, Name, __VA_ARGS__ ) }
#define GET_MEMBER_REFLECTION( ... ) GET_MEMBER_REFLECTION_0( __VA_ARGS__ )
#define BUILD_STRUCT_REFLECTED_MEMBER( Struct, Arg, ... ) GET_MEMBER_REFLECTION( Struct, UNWRAP_ ## Arg )  __VA_OPT__(,)
#define BUILD_STRUCT_REFLECTED_MEMBERS( Struct, ... ) array{ FOR_EACH_FORWARD_W_ARG( BUILD_STRUCT_REFLECTED_MEMBER, Struct, __VA_ARGS__ ) }

#define BUILD_STRUCT_BODY( Name, ... ) \
	BUILD_STRUCT_BEGIN( Name ) \
	BUILD_STRUCT_MEMBERS( __VA_ARGS__ ) \
	};

#define GET_ARGS_HASH( ... ) struct_hash< __VA_ARGS__ >::kHash

#define MAKE_STRUCT_TYPE_INFO( Type, Name, MembersHash, Types, ... ) \
template<> struct qw::get_type_info< Type >{ \
	constexpr static auto kMembers = const_map{ BUILD_STRUCT_REFLECTED_MEMBERS( Type, __VA_ARGS__ ) }; \
	constexpr static sStruct_Type_Info kInfo = { { sType_Info::eType::kStruct, MembersHash( UNWRAP( Types ) ), sizeof( Type ), Name, #Type }, kMembers }; \
	constexpr static bool      kValid = true; \
	};

#define REGISTER_STRUCT( Type, Types, Name, ... ) \
	MAKE_STRUCT_TYPE_INFO( Type, Name, GET_ARGS_HASH, PACK( Types ), __VA_ARGS__ ) \
	REGISTER_TYPE_INTERNAL( Type )

#define BUILD_TYPE_INFO( Name, ... )  \
	REGISTER_STRUCT( Name, GET_MEMBER_TYPES( __VA_ARGS__ ), #Name, __VA_ARGS__ )

#define MAKE_STRUCT( Name, ... ) \
	BUILD_STRUCT_BODY( Name, __VA_ARGS__ ) \
	BUILD_TYPE_INFO( Name, __VA_ARGS__ )

//MAKE_TYPE_INFO_DIRECT( Type, Name, = GET_ARGS_HASH, Types )