/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "cMatrix.h"
#include "cVector3.h"


namespace qw
{
	template < typename T > using cMatrix3x3 = cMatrix<3, 3, T>;

	typedef cMatrix3x3< float >	cMatrix3x3f;
	typedef cMatrix3x3< double >	cMatrix3x3d;

	template< typename T > class cMatrix< 3, 3, T >
	{
	public:
		cVector3<T> x = { T(1) , T(0) , T(0) };
		cVector3<T> y = { T(0) , T(1) , T(0) };
		cVector3<T> z = { T(0) , T(0) , T(1) };

		// Default constructor
		cMatrix() = default;
		constexpr cMatrix(const cVector3<T>& _x, const cVector3<T>& _y, const cVector3<T>& _z, const cVector3<T>& _w) : x(_x), y(_y), z(_z) {}


		// Hardcore but direct
		//constexpr Matrix operator*(const Matrix& _m) const { return { { x.x * _m.x.x + x.y * _m.y.x + x.z * _m.z.x + x.w * _m.w.x, x.x * _m.x.y + x.y * _m.y.y + x.z * _m.z.y + x.w * _m.w.y, x.x * _m.x.z + x.y * _m.y.z + x.z * _m.z.z + x.w * _m.w.z, x.x * _m.x.w + x.y * _m.y.w + x.z * _m.z.w + x.w * _m.w.w, }, { y.x * _m.x.x + y.y * _m.y.x + y.z * _m.z.x + y.w * _m.w.x, y.x * _m.x.y + y.y * _m.y.y + y.z * _m.z.y + y.w * _m.w.y, y.x * _m.x.z + y.y * _m.y.z + y.z * _m.z.z + y.w * _m.w.z, y.x * _m.x.w + y.y * _m.y.w + y.z * _m.z.w + y.w * _m.w.w, }, { z.x * _m.x.x + z.y * _m.y.x + z.z * _m.z.x + z.w * _m.w.x, z.x * _m.x.y + z.y * _m.y.y + z.z * _m.z.y + z.w * _m.w.y, z.x * _m.x.z + z.y * _m.y.z + z.z * _m.z.z + z.w * _m.w.z, z.x * _m.x.w + z.y * _m.y.w + z.z * _m.z.w + z.w * _m.w.w, }, { w.x * _m.x.x + w.y * _m.y.x + w.z * _m.z.x + w.w * _m.w.x, w.x * _m.x.y + w.y * _m.y.y + w.z * _m.z.y + w.w * _m.w.y, w.x * _m.x.z + w.y * _m.y.z + w.z * _m.z.z + w.w * _m.w.z, w.x * _m.x.w + w.y * _m.y.w + w.z * _m.z.w + w.w * _m.w.w, } }; }

		constexpr cMatrix operator-(void) const { return { -x, -y, -z, -w }; }

		constexpr cMatrix operator+(const cMatrix& _m) const { return { _m.x + x, _m.y + y, _m.z + z }; }
		constexpr cMatrix operator-(const cMatrix& _m) const { return { _m.x - x, _m.y - y, _m.z - z }; }
		constexpr cMatrix operator*(const cMatrix& _m) const { return { (_m.x * x.x) + (_m.y * x.y) + (_m.z * x.z), (_m.x * y.x) + (_m.y * y.y) + (_m.z * y.z), (_m.x * z.x) + (_m.y * z.y) + (_m.z * z.z) }; }

		constexpr cMatrix& operator+=(const cMatrix& _m) const { x += _m.x; y += _m.y; z += _m.z; w += _m.w; return *this; }
		constexpr cMatrix& operator-=(const cMatrix& _m) const { x -= _m.x; y -= _m.y; z -= _m.z; w -= _m.w; return *this; }
		constexpr cMatrix& operator*=(const cMatrix& _m) const { x = ((_m.x * x.x) + (_m.y * x.y) + (_m.z * x.z)); y = ((_m.x * y.x) + (_m.y * y.y) + (_m.z * y.z)); z = ((_m.x * z.x) + (_m.y * z.y) + (_m.z * z.z)); };
	};

	namespace Matrix4x4
	{
		template< typename T >
		constexpr cMatrix3x3< T > rotateX( const float& _a )
		{
			return {
				{ T(1), T(0),    T(0),     T(0) },
				{ T(0), cos(_a), -sin(_a), T(0) },
				{ T(0), sin(_a), cos(_a),  T(0) },
				{ T(0), T(0),    T(0),     T(1) },
			};
		}
		template< typename T >
		constexpr cMatrix3x3< T > rotateY( const float& _a )
		{
			return {
				{ cos(_a), T(0), -sin(_a), T(0) },
				{ T(0),    T(1), T(0),     T(0) },
				{ sin(_a), T(0), cos(_a),  T(0) },
				{ T(0),    T(0), T(0),     T(1) },
			};
		}
		template <typename T> constexpr cMatrix3x3<T> rotateZ(const float& _a)
		{
			return {
				{ cos(_a), -sin(_a), T(0), T(0) },
				{ sin(_a), cos(_a),  T(0), T(0) },
				{ T(0),    T(0),     T(1), T(0) },
				{ T(0),    T(0),     T(0), T(1) },
			};
		}
		// Rotates around XYZ according to euler angles
		template <typename T> constexpr cMatrix3x3<T> rotate(const cVector3<T>& _v)
		{
			return {
				{
					cos(_v.y) * cos(_v.z),
					sin(_v.x) * sin(_v.y) * cos(_v.z) - cos(_v.x) * sin(_v.z),
					cos(_v.x) * sin(_v.y) * cos(_v.z) + sin(_v.x) * sin(_v.z),
				},
				{
					cos(_v.y) * sin(_v.z),
					sin(_v.x) * sin(_v.y) * sin(_v.z) + cos(_v.x) * cos(_v.z),
					cos(_v.x) * sin(_v.y) * sin(_v.z) - sin(_v.x) * cos(_v.z),
				},
				{
					-sin(_v.y),
					sin(_v.x) * cos(_v.y),
					cos(_v.x) * cos(_v.y),
				},
				{ T(0), T(0), T(0),T(1)}
			};
		}
		// Rotates around axis
		template< typename T >
		cMatrix3x3< T > rotate( const cVector3< T >& _v, const float& _a )
		{

			const cVector3< T > _n = Vector3::Normalized( _v ); 
			return {
				{
					_v.x * _v.x * (1 - cos(_a)) + cos(_a),
					_v.x * _v.y * (1 - cos(_a)) - _v.z * sin(_a),
					_v.x * _v.z * (1 - cos(_a)) + _v.y * sin(_a), T(0)
				},
				{
					_v.x * _v.y * (1 - cos(_a)) + _v.z * sin(_a),
					_v.y * _v.y * (1 - cos(_a)) + cos(_a),
					_v.y * _v.z * (1 - cos(_a)) - _v.x * sin(_a), T(0)
				},
				{
					_v.x * _v.z * (1 - cos(_a)) - _v.y * sin(_a),
					_v.y * _v.z * (1 - cos(_a)) + _v.x * sin(_a),
					_v.z * _v.z * (1 - cos(_a)) + cos(_a), T(0)
				},
				{ T(0), T(0), T(0),T(1) },
			};
		}
	} // Matrix3x3
} // qw