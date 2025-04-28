/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <algorithm>

#include "cVector.h"

#include "VectorSwizzle.h"

namespace qw
{
	template <typename T> using cVector4 = cVector<4, T>;

	typedef cVector4<float>		cColor;
	typedef cVector4<float>		cVector4f;
	typedef cVector4<double>	cVector4d;
	typedef cVector4<int>		cVector4i;

	template <typename T>
	class cVector<4, T> // TODO: Make into a struct?
	{
	public:
		union { T x, r; };
		union { T y, g; };
		union { T z, b; };
		union { T w, a; };

		// Constructors:

		// Default constructor
		constexpr cVector(void) = default;
		// Sets all axis to x
		constexpr explicit cVector(const T _x) : x(_x), y(_x), z(_x), w(_x) {}
		// Sets all axis
		constexpr cVector(const T _x, const T _y, const T _z, const T _w) : x(_x), y(_y), z(_z), w(_w) {}
		template< typename T2 >
		constexpr cVector(const T2 _x, const T2 _y, const T2 _z, const T2 _w) : x( T( _x ) ), y( T( _y ) ), z( T( _z ) ), w( T( _w ) ){ }
		constexpr cVector( const cVector&  _other )          : x(_other.x), y(_other.y), z(_other.z), w(_other.w) {}
		constexpr cVector( const cVector&& _other ) noexcept : x(_other.x), y(_other.y), z(_other.z), w(_other.w) {}
		// Sets all axis (but using multiple types you psyco)
		template <typename T2, typename T3, typename T4, typename T5>
		constexpr cVector(const T2 _x, const T3 _y, const T4 _z, const T5 _w) : x(T(_x)), y(T(_y)), z(T(_z)), w(T(_w)) {}


		// Construction from other Vectors:

		// Construct using only a Vector2 (Z becomes 0 and W becomes 1)
		constexpr cVector( const cVector< 2, T >& _v ) : x( _v.x ), y( _v.y ), z( T(0) ), w( T(1) ){ }
		// Construct using a Vector2, Z and W
		explicit constexpr cVector( const cVector< 2, T >& _v, const T _z, const T _w ) :x( _v.x ), y( _v.y ), z( _z ), w( _w ){ }
		// Constructs using only a Vector3 (W becomes 1)
		constexpr cVector( cVector< 3, T > _v ) : x( _v.x ), y( _v.y ), z( _v.z ), w( T( 1 ) ){ }
		// Construct using a Vector3 and W
		explicit constexpr cVector( const cVector< 3, T >& _v, const T _w ) : x( _v.x ), y( _v.y ), z( _v.z ), w( _w ){ }

		// Construct using only a Vector2 (Z becomes 0 and W becomes 1)
		template <typename T2>
		constexpr cVector(const cVector<2, T2>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(T(0)), w(T(1)) {}
		// Construct using a Vector2, Z and W
		template <typename T2>
		explicit constexpr cVector(const cVector<2, T2>& _v, const T _z, const T _w) :x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(_z), w(_w) {}
		// Constructs using only a Vector3 (W becomes 1)
		template <typename T2>
		constexpr cVector(const cVector<3, T2>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)), w(T(1)) {}
		// Construct using a Vector3 and W
		template <typename T2>
		explicit constexpr cVector(const cVector<3, T2>& _v, const T _w) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)), w(_w) {}
		// Copies other Vector4
		template <typename T2>
		constexpr cVector(const cVector<4, T2>& _v) : x( static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)), w(static_cast<T>(_v.w)) {}
		// Construct from array/pointer
		explicit constexpr cVector(const T _p[4]) : x(_p[0]), y(_p[1]), z(_p[2]), w(_p[3]) {}

		~cVector( void ) = default;

		// Cast to other Vectors:
		template <typename T2>
		constexpr operator cVector< 2, T2 >(){ return cVector< 2, T2 >( *this ); }
		template <typename T2>
		constexpr operator cVector< 3, T2 >(){ return cVector< 3, T2 >( *this ); }
		template <typename T2>
		constexpr operator cVector< 4, T2 >(){ return cVector< 4, T2 >( *this ); }

		// Casting if color
		explicit constexpr operator uint32_t(){ return bgra(); }

		// Operators:
		constexpr cVector operator-(void) const { return { -x, -y, -z, -w }; }

		constexpr cVector& operator=(const cVector& _v)         { x = _v.x; y = _v.y; z = _v.z;   w = _v.w;   return *this; }

		constexpr cVector operator+(const cVector& _v) const { return { x + _v.x, y + _v.y, z + _v.z, w + _v.w }; }
		constexpr cVector operator-(const cVector& _v) const { return { x - _v.x, y - _v.y, z - _v.z, w - _v.w }; }
		constexpr cVector operator*(const cVector& _v) const { return { x * _v.x, y * _v.y, z * _v.z, w * _v.w }; }
		constexpr cVector operator/(const cVector& _v) const { return { x / _v.x, y / _v.y, z / _v.z, w / _v.w }; }

		template <typename T2>
		constexpr cVector operator*(const T2 _t) const { return { static_cast<T>(x * _t), static_cast<T>(y * _t), static_cast<T>(z * _t), static_cast<T>(w * _t) }; }
		template <typename T2>
		constexpr cVector operator/(const T2 _t) const { return { static_cast<T>(x / _t), static_cast<T>(y / _t), static_cast<T>(z / _t), static_cast<T>(w / _t) }; }

		constexpr cVector& operator++(void) { ++x; ++y; ++z; ++w; return *this; }
		constexpr cVector& operator--(void) { --x; --y; --z; --w; return *this; }

		constexpr cVector& operator+=(const cVector& _v) { x += _v.x; y += _v.y; z += _v.z; w += _v.w; return *this; }
		constexpr cVector& operator-=(const cVector& _v) { x -= _v.x; y -= _v.y; z -= _v.z; w -= _v.w; return *this; }
		constexpr cVector& operator*=(const cVector& _v) { x *= _v.x; y *= _v.y; z *= _v.z; w *= _v.w; return *this; }
		constexpr cVector& operator/=(const cVector& _v) { x += _v.x; y += _v.y; z /= _v.z; w /= _v.w; return *this; }

		template <typename T2>
		constexpr cVector& operator*=(const T2 _t) const { x *= static_cast<T>(_t); y *= static_cast<T>(_t); z *= static_cast<T>(_t); w *= static_cast<T>(_t); return *this; }
		template <typename T2>
		constexpr cVector& operator/=(const T2 _t) const { x /= static_cast<T>(_t); y /= static_cast<T>(_t); z /= static_cast<T>(_t); w *= static_cast<T>(_t); return *this; }

		constexpr bool operator==(const cVector& _v) { return (x == _v.x && y == _v.y && z == _v.z && w == _v.w); }
		constexpr bool operator!=(const cVector& _v) { return !(*this == _v); }
		constexpr T& operator[](const size_t _i) { return (&x)[_i]; }
		constexpr const T& operator[](const size_t _i) const { return (&x)[_i]; }

		// Vector Math

		// Gets the dot product of self
		constexpr	T				dot(void) const { return (x * x) + (y * y) + (z * z) + (w * w); }
		// Returns the dot product of self and other vector
		constexpr	T				dot(const cVector& _v) { return (x * _v.x) + (y * _v.y) + (z * _v.z) + (w * _v.w); }
		// Returns the length
		inline		T				length(void) { return Math::sqrt(dot()); }
		// Normalizes and returns self
		// (Original will be modified)
		inline		cVector&			normalize(void) { return normalize(length()); }
		// Normalizes after length and returns self
		// (Original will be modified)
		constexpr	cVector&			normalize(const T _l) { *this *= _l > T(0) ? (T(1) / _l) : T(0); return *this; }
		// Returns a makes out vector into normalized version of self
		inline		void                normalize( cVector& _out ) { return _out = cVector( *this ).normalize(length()); }
		// Returns the normalized direction from self towards position
		inline		cVector4<T>		dir(const cVector4<T>& _p) { return (_p - *this).normalize(); }

		constexpr uint32_t rgba( void )
		{
			return std::clamp( static_cast< uint32_t >( r * T( 0xff000000u ) ), 0u, 0xff000000u )
			     | std::clamp( static_cast< uint32_t >( g * T( 0x00ff0000u ) ), 0u, 0x00ff0000u )
			     | std::clamp( static_cast< uint32_t >( b * T( 0x0000ff00u ) ), 0u, 0x0000ff00u )
			     | std::clamp( static_cast< uint32_t >( a * T( 0x000000ffu ) ), 0u, 0x000000ffu );
		}

		constexpr uint32_t abgr( void )
		{
			return std::clamp( static_cast< uint32_t >( a * T( 0xff000000u ) ), 0u, 0xff000000u )
			     | std::clamp( static_cast< uint32_t >( b * T( 0x00ff0000u ) ), 0u, 0x00ff0000u )
			     | std::clamp( static_cast< uint32_t >( g * T( 0x0000ff00u ) ), 0u, 0x0000ff00u )
			     | std::clamp( static_cast< uint32_t >( r * T( 0x000000ffu ) ), 0u, 0x000000ffu );
		}

		constexpr uint32_t bgra( void )
		{
			return std::clamp( static_cast< uint32_t >( b * T( 0xff000000u ) ), 0u, 0xff000000u )
			     | std::clamp( static_cast< uint32_t >( g * T( 0x00ff0000u ) ), 0u, 0x00ff0000u )
			     | std::clamp( static_cast< uint32_t >( r * T( 0x0000ff00u ) ), 0u, 0x0000ff00u )
			     | std::clamp( static_cast< uint32_t >( a * T( 0x000000ffu ) ), 0u, 0x000000ffu );
		}

		SWIZZLE_ALL_VEC4

	};

	namespace Vector4
	{
		// Returns the dot product of two vectors
		template <typename T> constexpr	T			Dot(const cVector4<T>& _v) { return (_v.x * _v.x) + (_v.y * _v.y) + (_v.z * _v.z) + (_v.w * _v.w); }
		// Returns the dot product of two vectors
		template <typename T> constexpr	T			Dot(const cVector4<T>& _v, const cVector4<T>& _u) { return (_v.x * _u.x) + (_v.y * _u.y) + (_v.z * _u.z) + (_v.w * _u.w); }
		// Returns the distance between two positions
		template <typename T>			T			Distance(cVector4<T> _v1, cVector4<T> _v2) { return (_v1 - _v2).length(); }
		// Returns the normalized direction from position 1 towards position 2
		template <typename T> inline	cVector4<T>	Direction(const cVector4<T>& _p1, const cVector4<T>& _p2) { return (_p2 - _p1).normalize(); }
		// Returns projection between two values:
		template <typename T> inline	cVector4<T> Project(const cVector4<T>& _v, const cVector4<T>& _u) { return _u * (Dot(_v, _u) / _u.dot()); }
		// Returns a normalized version of vector after length
		template <typename T> inline	cVector4<T>	Normalized(const cVector4<T>& _v, const T _l) { return _v * (_l > T(0) ? (T(1) / _l) : T(0)); }
		// Returns a normalized version of vector
		template <typename T> inline	cVector4<T>	Normalized(const cVector4<T>& _v) { return Normalized(_v, Length(_v)); }

	} // Vector4::

	namespace Color
	{
		constexpr auto kWhite       = cColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		constexpr auto kBlack       = cColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		constexpr auto kTransparent = cColor{ 0.0f, 0.0f, 0.0f, 0.0f };

		constexpr auto kRed         = cColor{ 1.0f, 0.0f, 0.0f, 1.0f };
		constexpr auto kGreen       = cColor{ 0.0f, 1.0f, 0.0f, 1.0f };
		constexpr auto kBlue        = cColor{ 0.0f, 0.0f, 1.0f, 1.0f };

	} // Color::

} // qw::

template< typename Ty >
qw::cVector4< Ty > operator*( const Ty& _l, const qw::cVector4< Ty >& _r ){ return qw::cVector4< Ty >{ _r.x * _l, _r.y * _l, _r.z * _l, _r.w * _l }; }