/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Types/types.h"

#include "cVector.h"

#include "VectorSwizzle.h"

namespace qw
{
	template< typename T >
	using cVector2 = cVector< 2, T >;

	typedef cVector2< float >    cVector2f;
	typedef cVector2< double >   cVector2d;
	typedef cVector2< int32_t >  cVector2i;
	typedef cVector2< uint32_t > cVector2u;

	template< typename T >
	class cVector< 2, T >
	{
	public:
		union { T x, r, width;  };
		union { T y, g, height; };

		// Constructors:

		// Default constructor
		constexpr cVector( void ) = default;
		// Sets all axis to x
		constexpr explicit cVector( const T _x )								: x(_x),y(_x) {}
		// Sets all axis
		constexpr cVector( const T _x, const T _y )							: x(_x),y(_y) {}
		// Copy constructor
		constexpr cVector( const cVector& _other )							: x( _other.x ),y( _other.y ) {}
		// Copy constructor
		constexpr cVector( const cVector&& _other )	noexcept				: x( _other.x ),y( _other.y ) {}
		// Sets all axis (but using multiple types you psyco)
		template <typename T2, typename T3> constexpr cVector(const T2 _x, const T3 _y) : x(_x), y(_y) {}

		// Construction from other Vectors:

		// Copies Vector2
		template <typename T2>
		constexpr          cVector( const cVector<2, T2>& _v )	: x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {}
		// Get XY from Vector3
		template <typename T2>
		constexpr          cVector( const cVector<3, T2>& _v )	: x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {}
		// Get XY from Vector4
		template <typename T2>
		constexpr          cVector( const cVector<4, T2>& _v )	: x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)) {}
		// Construct from numbers in array/pointer
		constexpr explicit cVector( const T* _p[2]) : x(_p[0]), y(_p[1]) {}

		~cVector( void ) = default;

		// Cast to other Vectors:
		template <typename T2> operator cVector< 2, T2>() { return cVector< 2, T2>(*this); }
		template <typename T2> operator cVector< 3, T2>() { return cVector< 3, T2>(*this); }
		template <typename T2> explicit operator cVector< 4, T2>() { return cVector< 4, T2>(*this); }

		// Operators:
		constexpr cVector operator-(void) const { return { -x, -y }; }
		constexpr cVector& operator=(const cVector& _v) { x = _v.x; y = _v.y; return *this; }

		constexpr cVector operator+(const cVector& _v) const { return { x + _v.x, y + _v.y }; }
		constexpr cVector operator-(const cVector& _v) const { return { x - _v.x, y - _v.y }; }
		constexpr cVector operator*(const cVector& _v) const { return { x * _v.x, y * _v.y }; }
		constexpr cVector operator/(const cVector& _v) const { return { x / _v.x, y / _v.y }; }

		template <typename T2> cVector operator*(const T2 _t) const { return { static_cast<T>(x * _t), static_cast<T>(y * _t) }; }
		template <typename T2> cVector operator/(const T2 _t) const { return { static_cast<T>(x / _t), static_cast<T>(y / _t) }; }

		constexpr cVector& operator++(void) { ++x; ++y; return *this; }
		constexpr cVector& operator--(void) { --x; --y; return *this; }

		constexpr cVector& operator+=(const cVector& _v) { x += _v.x; y += _v.y; return *this; }
		constexpr cVector& operator-=(const cVector& _v) { x -= _v.x; y -= _v.y; return *this; }
		constexpr cVector& operator*=(const cVector& _v) { x *= _v.x; y *= _v.y; return *this; }
		constexpr cVector& operator/=(const cVector& _v) { x += _v.x; y += _v.y; return *this; }

		template <typename T2> cVector& operator*=(const T2 _t) const { x *= static_cast<T>(_t); y *= static_cast<T>(_t); return *this; }
		template <typename T2> cVector& operator/=(const T2 _t) const { x /= static_cast<T>(_t); y /= static_cast<T>(_t); return *this; }

		constexpr bool operator==(const cVector& _v) { return (x == static_cast<T>(_v.x) && y == static_cast<T>(_v.y)); }
		constexpr bool operator!=(const cVector& _v) { return !(*this == _v); }
		constexpr T& operator[](const size_t _i) { return (&x)[_i]; }
		constexpr const T& operator[](const size_t _i) const { return (&x)[_i]; }

		operator float()
		{
			return length<float>();
		}

		operator double()
		{
			return length<double>();
		}

		// Vector Math

		// Gets the dot product of self
		constexpr	T				dot			(void) const			{ return (x * x) + (y * y); }
		// Gets the dot product of self
		template <typename T2> constexpr T2		dot(void) const			{ return (x * x) + (y * y); }
		// Returns the dot product of self and other vector
		constexpr	T				dot			(const cVector& _v)		{ return (x * _v.x) + (y * _v.y); }
		// Returns the length
		inline		T				length		(void)					{ return Math::sqrt(dot()); }
		// Returns the length
		template <typename T2> inline T2 length	(void)					{ return (T2)Math::sqrt(dot<T2>()); }
		// Normalizes and returns the vector
		// (Original will be modified)
		inline		cVector&			normalize	(void)					{ return normalize(length()); }
		// Normalizes and returns the vector after length
		// (Original will be modified)
		constexpr	cVector&			normalize	(const T _l)			{ *this *= _l > T(0) ? (T(1) / _l) : T(0); return *this; }
		// Returns a new normalized version of self
		inline		cVector2<T>		normalized	(void)					{ return normalized(length()); }
		// Returns a new normalized version of self after length
		constexpr	cVector2<T>		normalized	(const T _l)			{ return *this * (_l > T(0) ? (T(1) / _l) : T(0)); }
		// Returns the normalized direction from self towards position
		inline		cVector2<T>		dir(const cVector2<T>& _p)	{ return (_p - *this).normalize(); }

		SWIZZLE_ALL_VEC2

	};

	namespace Vector2
	{
		// Returns the dot product of two vectors
		template <typename T> constexpr	T			Dot(const cVector2<T>& _v, const cVector2<T>& _u)			{ return (_v.x * _u.x) + (_v.y * _u.y); }
		// Returns the distance between two positions
		template <typename T>			T			Distance(cVector2<T> _v1, cVector2<T> _v2)					{ return (_v1 - _v2).length(); }
		// Returns the normalized direction from position 1 towards position 2
		template <typename T> inline	cVector2<T>	Direction(const cVector2<T>& _p1, const cVector2<T>& _p2)	{ return (_p2 - _p1).normalize(); }
		// Returns the cross product between two vectors (Have fun I guess?)
		template <typename T> inline	cVector<3,T>	Cross(const cVector2<T>& _v, const cVector2<T>& _u)			{ return { T(0), T(0), _v.x * _u.y - _v.y * _u.x }; }
		// Returns projection between two values:
		template <typename T> inline	cVector2<T> Project(const cVector2<T>& _v, const cVector2<T>& _u)		{ return _u * (Dot(_v,_u) / _u.dot()); }
		// Returns a normalized version of vector after length
		template <typename T> inline	cVector2<T>	Normalized(const cVector2<T>& _v, const T _l)				{ return _v * (_l > T(0) ? (T(1) / _l) : T(0)); }
		// Returns a normalized version of vector
		template <typename T> inline	cVector2<T>	Normalized(const cVector2<T>& _v)							{ return Normalized(_v, Length(_v)); }
	}
}

template< typename Ty >
qw::cVector2< Ty > operator*( const Ty& _l, const qw::cVector2< Ty >& _r ){ return qw::cVector2< Ty >{ _r.x * _l, _r.y * _l }; }

constexpr auto a = sizeof( qw::cVector2f );
