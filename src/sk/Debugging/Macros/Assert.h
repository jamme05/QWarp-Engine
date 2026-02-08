/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Debugging/Severity.h>
#include <sk/Debugging/Macros/Text.h>
#include <sk/Macros/Manipulation.h>

#define NOTHING void()
#define NOT_NOTHING

// Safety https://clang.llvm.org/docs/LanguageExtensions.html
// More platform detection: https://stackoverflow.com/a/1959615
// TODO: Move platform detection macros to its own file.
#ifndef __has_builtin
#define __has_builtin(x) 0
#define HAS_BUILTIN_MACRO 0
#else // !defined( __has_builtin )
#define HAS_BUILTIN_MACRO 1
#endif // defined( __has_builtin )

#ifdef _MSC_VER
#define SK_HAS_MSC 1
#else // defined( _MSC_VER )
#define SK_HAS_MSC 0
#endif // !defined( _MSC_VER )

// Good reference: https://stackoverflow.com/a/49079078
#if __has_builtin( __builtin_debugtrap )
#define SK_BREAK __builtin_debugtrap()
#elif SK_HAS_MSC // __has_builtin( __builtin_debugtrap )
#define SK_BREAK __debugbreak()
#endif // !__has_builtin( __builtin_debugtrap )

// Fallback for linux
#ifndef SK_BREAK
#include <csignal>
#define SK_BREAK { ( void ) std::raise( SIGINT ); }
#endif // !defined( SK_BREAK )

// TODO: Make the passthrough only cover the debug print and not the if statement for methods returning.
#define SK_ASSERT_TEMPLATE( Condition, Method, Text, ... ) \
    if( Condition ){ [[ unlikely ]] EXTRACT_TEXT( Method, Text ); __VA_ARGS__ }

#define SK_ASSERT_TEMPLATE_IF_RET( Method, Condition, Severity, Text, ... ) \
    SK_PASSTHROUGH( Severity, SK_ASSERT_TEMPLATE( Condition, Method, Text, __VA_OPT__( __VA_ARGS__ ; ) ) )

#define SK_ASSERT_TEMPLATE_IF( Method, Condition, Severity, Text, ... ) \
    SK_PASSTHROUGH( Severity, SK_ASSERT_TEMPLATE( Condition, Method, Text, __VA_ARGS__ ) )

#define SK_CONST_ASSERT_TEMPLATE_IF_RET( Method, Condition, Text, ... ) \
    if constexpr( Condition ){ [[ unlikely ]] EXTRACT_TEXT( Method, Text ); return __VA_OPT__( __VA_ARGS__ ; ) }

#define SK_CONST_ASSERT_TEMPLATE_IF( Method, Condition, Text, ... ) \
    if constexpr( Condition ){ [[ unlikely ]] EXTRACT_TEXT( Method, Text ); __VA_ARGS__ }

// Warnings

// TODO: Maybe move to its own file?
// If the Condition is true, an error message will be printed and the function will return if.
#define SK_WARN_IF_RET( Severity, Condition, Text, ... ) \
    SK_ASSERT_TEMPLATE_IF_RET( WARN, Condition, Severity, Text, return __VA_ARGS__ )

// If the Condition is false, an error message will be printed and the function will return.
#define SK_WARN_IFN_RET( Severity, Condition, Text, ... ) \
    SK_WARN_IF_RET( Severity, NOT( Condition ), Text, __VA_ARGS__ )

#define SK_WARN_IF( Severity, Condition, Text ) \
    SK_ASSERT_TEMPLATE_IF( WARN, Condition, Severity, Text )

#define SK_WARN_IFN( Severity, Condition, Text ) \
    SK_WARN_IF( Severity, NOT( Condition ), Text )

#define SK_CONST_WARN_IF_RET( Severity, Condition, Text, ... ) \
    SK_PASSTHROUGH_RET( Severity, SK_CONST_ASSERT_TEMPLATE_IF_RET( WARN, Condition, Text, __VA_ARGS__ ) )

// If the Condition is false, an error message will be printed and the function will return.
#define SK_CONST_WARN_IFN_RET( Severity, Condition, Text, ... ) \
    SK_WARN_IF_RET( Severity, NOT( Condition ), Text, __VA_ARGS__ )

#define SK_CONST_WARN_IF( Severity, Condition, Text ) \
    SK_PASSTHROUGH( Severity, SK_CONST_ASSERT_TEMPLATE_IF( WARN, Condition, Text ) )

#define SK_CONST_WARN_IFN( Severity, Condition, Text ) \
    SK_WARN_IF( Severity, NOT( Condition ), Text )

// Breaks

#define SK_BREAK_IF( Severity, Condition, Text ) \
    SK_ASSERT_TEMPLATE_IF( WARN, Condition, Severity, Text, SK_BREAK; )

#define SK_BREAK_IFN( Severity, Condition, Text ) \
    SK_BREAK_IF( Severity, NOT( Condition ), Text )

#define SK_BREAK_RET_IF( Severity, Condition, Text, ... ) \
    SK_ASSERT_TEMPLATE_IF_RET( WARN, Condition, Severity, Text, SK_BREAK; return __VA_ARGS__ )

#define SK_BREAK_RET_IFN( Severity, Condition, Text, ... ) \
    SK_BREAK_RET_IF( Severity, NOT( Condition ), Text, __VA_ARGS__ )

// Errors

/**
 * Equal to:
 * if( Condition ) SK_FATAL( Text, Formatting );
 * 
 * @param Condition If the condition is true it quits the app.
 * @param Text The text to be displayed. See Debugging/Text.h
 */
#define SK_ERR_IF( Condition, Text ) \
    if( Condition ){ [[ unlikely ]] EXTRACT_TEXT( ERR, Text ); }

#define SK_ERR_IFN( Condition, Text ) \
    SK_ERR_IF( NOT( Condition ), Text )
