/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Debugging/Macros/Warning.h>
#include <sk/Macros/Manipulation.h>
#include <sk/Misc/Print.h>

#define THIS_T()
#define THIS_T_IS_TEXT ,1

#define RAW_TEXT( ... ) sk::format( __VA_ARGS__ )
#define PRINT_TEXT( ... ) sk::print( __VA_ARGS__ )
#define PRINTLN_TEXT( ... ) sk::println( __VA_ARGS__ )
#define WARN_TEXT( ... ) sk::println( stderr, __VA_ARGS__ )
#define BREAK_TEXT( ... ) SK_BREAK; WARN_TEXT( __VA_ARGS__ )
#define ERR_TEXT( ... ) SK_FATAL( __VA_ARGS__ )
#define TEXT( ... ) _TEXT( __VA_ARGS__ ) THIS_T

#define EXTRACT_FORMAT_TEXT( Text ) CONCAT( EXTRACT_FORMAT_TEXT_TYPE_, EXTRACT_FORMAT_TYPE( CONCAT( IS, Text ) ) ) ( Text )

#define UNPACK_TEXT_1_1( Text ) Text
#define UNPACK_TEXT_1_0( Literal ) TEXT( Literal )

#define UNPACK_TEXT_0( Text, Is_Text ) CONCAT( UNPACK_TEXT_1_, SECOND( Is_Text, 0 ) )( Text )
#define UNPACK_TEXT( Text ) UNPACK_TEXT_0( Text, CONCAT( Text, _IS_TEXT ) )

#define EXTRACT_TEXT( Method, Text ) CONCAT( Method, UNPACK_TEXT( Text ) )()
