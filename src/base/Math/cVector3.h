/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "cVector.h"

#include "VectorSwizzle.h"

namespace qw
{

	template <typename T> using cVector3 = cVector<3, T>;

	typedef cVector3<float>		cVector3f;
	typedef cVector3<double>	cVector3d;
	typedef cVector3<int>		cVector3i;

	template <typename T>
	class cVector<3, T>
	{
	public:
		union { T x = T( 0 ), r, width;  };
		union { T y = T( 0 ), g, height; };
		union { T z = T( 0 ), b, depth;  };

		// Constructors:

		// Default constructor
		constexpr cVector(void) = default;
		// Sets all axis to x
		constexpr explicit cVector(const T _x) : x(_x), y(_x), z(_x) {}
		// Sets all axis
		constexpr cVector(const T _x, const T _y, const T _z) : x(_x), y(_y), z(_z) {}
		// Sets all axis (but using multiple types you psyco)
		template <typename T2, typename T3, typename T4> constexpr cVector(const T2 _x, const T3 _y, const T4 _z) : x(static_cast<T>(_x)), y(static_cast<T>(_y)), z(static_cast<T>(_z)) {}


		// Construction from other Vectors:

		// Construct using only a Vector2 (Z becomes 0)
		template <typename T2>
		constexpr cVector(const cVector<2, T2>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(0) {}
		// Construct using a Vector2 and Z
		template <typename T2>
		constexpr explicit cVector(const cVector<2, T2>& _v, const T _z) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(_z) {}
		// Copies other Vector3
		template <typename T2>
		constexpr cVector(const cVector<3, T2>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)) {}
		// Get XYZ from Vector4
		template <typename T2>
		constexpr cVector(const cVector<4, T2>& _v) : x(static_cast<T>(_v.x)), y(static_cast<T>(_v.y)), z(static_cast<T>(_v.z)) {}
		// Construct from array/pointer
		constexpr explicit cVector(const T* _p[3]) : x(_p[0]), y(_p[1]), z(_p[2]) {}

		// Cast to other Vectors:
		explicit constexpr operator cVector< 2, T>() { return cVector< 2, T>(*this); }
		explicit constexpr operator cVector< 4, T>() { return cVector< 4, T>(*this); }

		template <typename T2>
		explicit constexpr operator cVector< 2, T2>() { return cVector< 2, T2>(*this); }
		template <typename T2>
		explicit constexpr operator cVector< 3, T2>() { return cVector< 3, T2>(*this); } // Infinite loop?
		template <typename T2>
		explicit constexpr operator cVector< 4, T2>() { return cVector< 4, T2>(*this); }

		// Operators:
		constexpr cVector  operator-(void) const { return { -x, -y, -z }; }
		constexpr cVector& operator=(const cVector& _v){ x = _v.x; y = _v.z; z = _v.z; return *this; }


		constexpr cVector operator+(const cVector& _v) const { return { x + _v.x, y + _v.y, z + _v.z }; }
		constexpr cVector operator-(const cVector& _v) const { return { x - _v.x, y - _v.y, z - _v.z }; }
		constexpr cVector operator*(const cVector& _v) const { return { x * _v.x, y * _v.y, z * _v.z }; }
		constexpr cVector operator/(const cVector& _v) const { return { x / _v.x, y / _v.y, z / _v.z }; }
		constexpr cVector operator*(const T&       _t) const { return { x * _t,   y * _t,   z * _t   }; }

		template <typename T2> cVector operator*(const T2 _t) const { return { static_cast<T>(x * _t), static_cast<T>(y * _t), static_cast<T>(z * _t) }; }

		constexpr cVector& operator++(void) { ++x; ++y; ++z; return *this; }
		constexpr cVector& operator--(void) { --x; --y; --z; return *this; }

		constexpr cVector& operator+=(const cVector& _v){ x += _v.x; y += _v.y; z += _v.z; return *this; }
		constexpr cVector& operator-=(const cVector& _v){ x -= _v.x; y -= _v.y; z -= _v.z; return *this; }
		constexpr cVector& operator*=(const cVector& _v){ x *= _v.x; y *= _v.y; z *= _v.z; return *this; }
		constexpr cVector& operator/=(const cVector& _v){ x += _v.x; y += _v.y; z /= _v.z; return *this; }
		constexpr cVector& operator*=(const T&       _t){ x *= _t;   y *= _t;   z *= _t;   return *this; }

		template <typename T2> cVector& operator*=(const T2 _t) const { x *= static_cast<T>(_t); y *= static_cast<T>(_t); z *= static_cast<T>(_t); return *this; }

		constexpr bool operator==(const cVector& _v) { return (x == _v.x && y == _v.y && z == _v.z); }
		constexpr bool operator!=(const cVector& _v) { return !(*this == _v); }
		constexpr T& operator[](const size_t _i) { return (&x)[_i]; }
		constexpr const T& operator[](const size_t _i) const { return (&x)[_i]; }

		// Vector Math

		// Gets the dot product of self
		constexpr	T				dot(void) const { return (x * x) + (y * y) + (z * z); }
		// Returns the dot product of self and other vector
		constexpr	T				dot(const cVector& _v) { return (x * _v.x) + (y * _v.y) + (z * _v.z); }
		// Returns the length
		inline		T				length(void) { return Math::sqrt(dot()); }
		// Normalizes and returns the vector
		// (Original will be modified)
		inline		cVector&			normalize(void) { return normalize(length()); }
		// Normalizes and returns the vector after length
		// (Original will be modified)
		constexpr	cVector&			normalize(const T _l) { *this *= _l > T(0) ? (T(1) / _l) : T(0); return *this; }
		// Returns a new normalized version of self
		inline		cVector			normalized(void) { return normalized(length()); }
		// Returns a new normalized version of self after length
		constexpr	cVector3<T>		normalized(const T _l) { return *this * (_l > T(0) ? (T(1) / _l) : T(0)); }
		// Returns the normalized direction from self towards position
		inline		cVector3<T>		dir(const cVector3<T>& _p) { return (_p - *this).normalize(); }

		SWIZZLE_ALL_VEC3
	};

	namespace Vector3
	{
		// Returns the dot product of two vectors
		template <typename T> constexpr	T			Dot(const cVector3<T>& _v) { return (_v.x * _v.x) + (_v.y * _v.y) + (_v.z * _v.z); }
		// Returns the dot product of two vectors
		template <typename T> constexpr	T			Dot(const cVector3<T>& _v, const cVector3<T>& _u) { return (_v.x * _u.x) + (_v.y * _u.y) + (_v.z * _u.z); }
		// Returns the distance between two positions
		template <typename T>			T			Distance(cVector3<T> _v1, cVector3<T> _v2) { return (_v1 - _v2).length(); }
		// Returns the normalized direction from position 1 towards position 2
		template <typename T> inline	cVector3<T>	Direction(const cVector3<T>& _p1, const cVector3<T>& _p2) { return (_p2 - _p1).normalize(); }
		// Returns the cross product from two vectors
		template <typename T> inline	cVector3<T>	Cross(const cVector3<T>& _v, const cVector3<T>& _u) { return { _v.y * _u.z - _v.z * _u.y, _v.z * _u.x - _v.x * _u.z, _v.x * _u.y - _v.y * _u.x }; }
		// Returns projection between two values:
		template <typename T> inline	cVector3<T> Project(const cVector3<T>& _v, const cVector3<T>& _u) { return _u * (Dot(_v, _u) / _u.dot()); }
		// Return length of vector
		template <typename T> inline	T			Length(const cVector3<T>& _v) { return Math::sqrt(Dot(_v)); }
		// Returns a normalized version of vector after length
		template <typename T> inline	cVector3<T>	Normalized(const cVector3<T>& _v, const T _l) { return _v * (_l > T(0) ? (T(1) / _l) : T(0)); }
		// Returns a normalized version of vector
		template <typename T> inline	cVector3<T>	Normalized(const cVector3<T>& _v) { return Normalized(_v,Length(_v)); }

	} // Vector3::

	static constexpr auto kZero  =  cVector3f{ 0.0f, 0.0f, 0.0f };
	static constexpr auto kOne   =  cVector3f{ 1.0f, 1.0f, 1.0f };
	static constexpr auto kUp    =  cVector3f{ 0.0f, 1.0f, 0.0f };
	static constexpr auto kRight =  cVector3f{ 1.0f, 0.0f, 0.0f };
	static constexpr auto kAt    =  cVector3f{ 0.0f, 0.0f, 1.0f };
	static constexpr auto kDown  = -cVector3f{ 0.0f, 1.0f, 0.0f };
	static constexpr auto kLeft  = -cVector3f{ 1.0f, 0.0f, 0.0f };
	static constexpr auto kBack  = -cVector3f{ 0.0f, 0.0f, 1.0f };

} // qw::

template< typename Ty >
qw::cVector3< Ty > operator*( const Ty& _l, const qw::cVector3< Ty >& _r ){ return qw::cVector3< Ty >{ _r.x * _l, _r.y * _l, _r.z * _l }; }