/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Macros/for_each.h"

#define VEC2_COMBINATION( X, Y )       X##Y
#define VEC3_COMBINATION( X, Y, Z )    X##Y##Z
#define VEC4_COMBINATION( X, Y, Z, W ) X##Y##Z##W

#define MAKE_SWIZZLE_VEC2( X, Y )       constexpr cVector< 2, T > VEC2_COMBINATION( X, Y )      ( void ) const { return cVector< 2, T >( X, Y       ); }
#define MAKE_SWIZZLE_VEC3( X, Y, Z )    constexpr cVector< 3, T > VEC3_COMBINATION( X, Y, Z )   ( void ) const { return cVector< 3, T >( X, Y, Z    ); }
#define MAKE_SWIZZLE_VEC4( X, Y, Z, W ) constexpr cVector< 4, T > VEC4_COMBINATION( X, Y, Z, W )( void ) const { return cVector< 4, T >( X, Y, Z, W ); }

#define SWIZZLE_VEC2_DIRECT( X, NX, NY ) \
MAKE_SWIZZLE_VEC2( X, NX )               \
MAKE_SWIZZLE_VEC2( X, NY )

#define SWIZZLE_VEC3_DIRECT( X, Y, NX, NY, NZ ) \
MAKE_SWIZZLE_VEC3( X, Y, NX )                   \
MAKE_SWIZZLE_VEC3( X, Y, NY )                   \
MAKE_SWIZZLE_VEC3( X, Y, NZ )

#define SWIZZLE_VEC4_DIRECT_C( X, Y, Z, NX, NY, NZ, NW ) \
MAKE_SWIZZLE_VEC4( X, Y, Z, NX )                         \
MAKE_SWIZZLE_VEC4( X, Y, Z, NY )                         \
MAKE_SWIZZLE_VEC4( X, Y, Z, NZ )                         \
MAKE_SWIZZLE_VEC4( X, Y, Z, NW )

#define SWIZZLE_VEC4_DIRECT_B( X, Y, NX, NY, NZ, NW ) \
SWIZZLE_VEC3_DIRECT( X, Y, NX, NY, NZ )               \
MAKE_SWIZZLE_VEC3( X, Y, NW )                         \
SWIZZLE_VEC4_DIRECT_C( X, Y, NX, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_C( X, Y, NY, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_C( X, Y, NZ, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_C( X, Y, NW, NX, NY, NZ, NW )     \

#define SWIZZLE_VEC4_DIRECT_A( X, NX, NY, NZ, NW ) \
SWIZZLE_VEC4_DIRECT_B( X, NX, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_B( X, NY, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_B( X, NZ, NX, NY, NZ, NW )     \
SWIZZLE_VEC4_DIRECT_B( X, NW, NX, NY, NZ, NW )     \

// Creates a swizzle combining X with NX and NY
#define SWIZZLE_VEC2( X, NX, NY ) \
MAKE_SWIZZLE_VEC2( X, NX )        \
MAKE_SWIZZLE_VEC2( X, NY )        \

// Creates a swizzle combining X with NX, NY and NZ
#define SWIZZLE_VEC3( X, NX, NY, NZ )    \
SWIZZLE_VEC2_DIRECT( X, NX, NY )         \
MAKE_SWIZZLE_VEC2( X, NZ )               \
SWIZZLE_VEC3_DIRECT( X, NX, NX, NY, NZ ) \
SWIZZLE_VEC3_DIRECT( X, NY, NX, NY, NZ ) \
SWIZZLE_VEC3_DIRECT( X, NZ, NX, NY, NZ ) \

// Creates a swizzle combining X with NX, NY, NZ and NW
#define SWIZZLE_VEC4( X, NX, NY, NZ, NW ) \
SWIZZLE_VEC2_DIRECT( X, NX, NY )          \
MAKE_SWIZZLE_VEC2( X, NZ )                \
MAKE_SWIZZLE_VEC2( X, NW )                \
SWIZZLE_VEC4_DIRECT_A( X, NX, NY, NZ, NW )

// Container of types to allow for each
#define SWIZZLE_VEC2_XY( X )   SWIZZLE_VEC2( X, x, y )
// Container of types to allow for each
#define SWIZZLE_VEC3_XYZ( X )  SWIZZLE_VEC3( X, x, y, z )
// Container of types to allow for each
#define SWIZZLE_VEC4_XYZW( X ) SWIZZLE_VEC4( X, x, y, z, w )

// Container of types to allow for each
#define SWIZZLE_VEC2_RG( X )   SWIZZLE_VEC2( X, r, g )
// Container of types to allow for each
#define SWIZZLE_VEC3_RGB( X )  SWIZZLE_VEC3( X, r, g, b )
// Container of types to allow for each
#define SWIZZLE_VEC4_RGBA( X ) SWIZZLE_VEC4( X, r, g, b, a )

// Creates all swizzle functions for XY
#define SWIZZLE_XY   FOR_EACH( SWIZZLE_VEC2_XY,   x, y )
// Creates all swizzle functions for XYZ
#define SWIZZLE_XYZ  FOR_EACH( SWIZZLE_VEC3_XYZ,  x, y, z )
// Creates all swizzle functions for XYZW
#define SWIZZLE_XYZW FOR_EACH( SWIZZLE_VEC4_XYZW, x, y, z, w )

// Creates all swizzle functions for RG
#define SWIZZLE_RG   FOR_EACH( SWIZZLE_VEC2_RG,   r, g )
// Creates all swizzle functions for RGB
#define SWIZZLE_RGB  FOR_EACH( SWIZZLE_VEC3_RGB,  r, g, b )
// Creates all swizzle functions for RGBA
#define SWIZZLE_RGBA FOR_EACH( SWIZZLE_VEC4_RGBA, r, g, b, a )

#ifdef ENABLE_SWIZZLE
// All swizzles for the cVector2
#define SWIZZLE_ALL_VEC2 SWIZZLE_XY   SWIZZLE_RG
// All swizzles for the cVector3
#define SWIZZLE_ALL_VEC3 SWIZZLE_XYZ  SWIZZLE_RGB
// All swizzles for the cVector4
#define SWIZZLE_ALL_VEC4 SWIZZLE_XYZW SWIZZLE_RGBA
#else // ENABLE_SWIZZLE
// All swizzles for the cVector2
#define SWIZZLE_ALL_VEC2
// All swizzles for the cVector3
#define SWIZZLE_ALL_VEC3
// All swizzles for the cVector4
#define SWIZZLE_ALL_VEC4
#endif // !ENABLE_SWIZZLE