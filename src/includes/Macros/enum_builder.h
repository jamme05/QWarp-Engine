
#pragma once

#include "for_each.h"
#include "manipulation.h"

#define ALL_ARG_FOR_EACH(macro, Arg, ...)                    \
__VA_OPT__(EXPAND(A_FOR_EACH_HELPER(macro, Arg, __VA_ARGS__)))
#define A_FOR_EACH_HELPER(macro, Arg, a1, ...)                         \
macro(Arg,a1)                                                     \
__VA_OPT__(A_FOR_EACH_AGAIN PARENS (macro, Arg, __VA_ARGS__))
#define A_FOR_EACH_AGAIN() A_FOR_EACH_HELPER

#define TYPE_ENUM_2( Name, Type ) enum Name : Type
#define TYPE_ENUM_1( Name ) enum Name : uint8_t
#define TYPE_ENUM( ... ) CONCAT( TYPE_ENUM_, VARGS( __VA_ARGS__ ) )( __VA_ARGS__ )
#define TYPE_ENUMCLASS( ... ) TYPE_ENUM( class __VA_ARGS__ )

#define ENUM( ... ) _ENUM ( __VA_ARGS__ )
#define ENUMCLASS( ... ) _ENUMCLASS ( __VA_ARGS__ )

#define RAW_ENUM( Name, ... ) Name
#define RAW_ENUMCLASS( Name, ... ) Name

#define E( ... ) _E( __VA_ARGS__ )

#define NAME_E( Name, ... ) Name
#define STR_NAME_E( Name, ... ) #Name

// TODO: Add max check of some sort
#define VALUE_E_3( Name, Value, ... ) Value
#define VALUE_E_2( Name, Value ) Value
#define VALUE_E_1( Name )
#define VALUE_E( ... ) CONCAT( VALUE_E_, VARGS( __VA_ARGS__ ) )( __VA_ARGS__ )

#define UNPACK_SAFE_E( Name, ... ) __VA_OPT__(,) __VA_ARGS__

#define CREATE_VALUE_METADATA_TYPE( Type ) \
struct sValue \
{ const Type Value; \
const char* Name; \
const char* DisplayName; \
};
#include <cstdint>

template< class... Args >
constexpr int get_safe_enum_value( const char*, const int _counter, Args... ){ return _counter; };
template< class... Args >
constexpr int get_safe_enum_value( const int _value, const int, Args... ){ return _value; };
constexpr int get_safe_enum_value( const int _value ){ return _value; };

#define GET_SAFE_VALUE_2( Counter, ... ) = get_safe_enum_value( __VA_ARGS__ __VA_OPT__(,) Counter )
#define GET_SAFE_VALUE_1( Counter, Value )
#define GET_SAFE_VALUE( ... ) CONCAT( GET_SAFE_VALUE_, VARGS( __VA_ARGS__ ) )( __VA_ARGS__ )

#define VALID_E( ... ) ,1

#define IS_VALID_1( ... ) __VA_ARGS__ // Forward
#define IS_VALID_0( ... ) _E( __VA_ARGS__ ) // Create _E Object

#define IS_E_TYPE( ... ) SECOND( __VA_ARGS__, 0 )
#define UNWRAP_E_VALUE( Value ) CONCAT( IS_VALID_, IS_E_TYPE( CONCAT( VALID, Value ) ) ) ( Value )

#define MAKE_ENUM_VALUE_1( Value, Counter ) NAME ## Value GET_SAFE_VALUE( Counter, VALUE ## Value ),
#define MAKE_ENUM_VALUE( Value, Counter ) MAKE_ENUM_VALUE_1( Value, Counter )
#define UNPACK_ENUM_VALUE( Value, Counter ) MAKE_ENUM_VALUE( UNWRAP_E_VALUE( Value ), Counter )

#define BUILD_ENUM_BODY( Type, ... ) \
TYPE ## Type { COUNTER_FOR_EACH( UNPACK_ENUM_VALUE, 0, __VA_ARGS__ ) }

#define INFO_GETTER_IF_1( Type, Value, Counter ) if( RAW ## Type :: NAME ## Value == _v ) return Values[ Counter ];
#define INFO_GETTER_IF_0( Type, Value, Counter ) INFO_GETTER_IF_1( Type, Value, Counter )
#define INFO_GETTER_IF( Type, Value, Counter ) INFO_GETTER_IF_0( Type, UNWRAP_E_VALUE( Value ), Counter )

#define BUILD_INFO_GETTER( Type, ... ) \
static constexpr const sValue& get_enum_info( const RAW ## Type _v ){ \
COUNTER_ARG_FOR_EACH( INFO_GETTER_IF, Type, 0, __VA_ARGS__ ) \
return kInvalid; \
}                \

#define ENUM_VALUE_METADATA_1( Type, Value ) enum_value_creator< RAW ## Type, sValue >( RAW ## Type :: NAME ## Value, STR_NAME ## Value UNPACK_SAFE ## Value ),
#define ENUM_VALUE_METADATA_0( Type, Value ) ENUM_VALUE_METADATA_1( Type, Value )
#define ENUM_VALUE_METADATA( Type, Value ) ENUM_VALUE_METADATA_0( Type, UNWRAP_E_VALUE( Value ) )

#define BUILD_ENUM_METADATA( Type, ... )    \
struct CONCAT( RAW ## Type, _Metadata )     \
{ CREATE_VALUE_METADATA_TYPE( RAW ## Type ) \
BUILD_INFO_GETTER( Type, __VA_ARGS__ ) \
static constexpr sValue Values[]{ ALL_ARG_FOR_EACH( ENUM_VALUE_METADATA, Type, __VA_ARGS__ ) }; \
static constexpr sValue kInvalid = enum_value_creator< RAW ## Type, sValue >( static_cast< RAW ## Type >( 0 ), "Invalid" ); \
} // Removed semicolon to give the c++ feel ehe

// Macro used for getting the metadata type. Recommended to use in case of future changes.
#define META( Type ) Type ## _Metadata
#define ENUM_INFO( Type, Value ) META( Type ) :: get_enum_info( Value )

// TODO: Documentation
/**
 * 
 * @param Type ENUM/ENUMCLASS ( Name, [ Size ] )
 * @param ...  Works?
 *
**/
#define ENUM_CREATOR( Type, ... ) \
BUILD_ENUM_BODY( Type, __VA_ARGS__ ); \
BUILD_ENUM_METADATA( Type, __VA_ARGS__ )

template< class ETy, class Ty, class... Args >
constexpr Ty enum_value_creator( ETy _value, const char* _name, const char* _display_name, Args... )
{
	return { _value, _name, _display_name };
} // enum_value_creator

template< class ETy, class Ty, class... Args >
constexpr Ty enum_value_creator( ETy _value, const char* _name, const size_t, const char* _display_name, Args... )
{
	return { _value, _name, _display_name }; // Forward in case of future changes.
} // enum_value_creator

template< class ETy, class Ty, class... Args >
constexpr Ty enum_value_creator( ETy _value, const char* _name, const int, const char* _display_name, Args... )
{
	return { _value, _name, _display_name }; // Forward in case of future changes.
} // enum_value_creator

template< class ETy, class Ty >
constexpr Ty enum_value_creator( ETy _value, const char* _name )
{
	return { _value, _name, _name };
} // enum_value_creator

template< class ETy, class Ty >
constexpr Ty enum_value_creator( ETy _value, const char* _name, const size_t )
{
	return { _value, _name, _name }; // Forward in case of future changes.
} // enum_value_creator

template< class ETy, class Ty >
constexpr Ty enum_value_creator( ETy _value, const char* _name, const int )
{
	return { _value, _name, _name }; // Forward in case of future changes.
} // enum_value_creator


ENUM_CREATOR( ENUM( eExample1 ),
	E( kValueEx, 0x00 ),
	E( kDisplayNameEx, "Display Name" ),
	E( kBothEx, 3, "Wha" ),
	E( kNormal )
);

ENUM_CREATOR( ENUMCLASS( eExample2 ),
	E( kFirst ),
	E( kSecond ),
	E( kThird )
);

ENUM_CREATOR( ENUMCLASS( eExample3, uint16_t ),
	kFirst,
	kSecond,
	kThird
);