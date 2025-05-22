/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <pmmintrin.h>

#include "cMatrix.h"

#include "cVector3.h"
#include "cVector4.h"

#include "Matrix_helper.h"

namespace qw
{
	template <typename T> using cMatrix4x4 = cMatrix<4, 4, T>;

	typedef cMatrix4x4<float>	cMatrix4x4f;
	typedef cMatrix4x4<double>	cMatrix4x4d;

	template <typename T> class cMatrix<4, 4, T>
	{
	public:
		union{ cVector4<T> x = { T(1) , T(0) , T(0) , T(0) }, right;    };
		union{ cVector4<T> y = { T(0) , T(1) , T(0) , T(0) }, up;       };
		union{ cVector4<T> z = { T(0) , T(0) , T(1) , T(0) }, front;    };
		union{ cVector4<T> w = { T(0) , T(0) , T(0) , T(1) }, position; };

		// Default constructor
		cMatrix() = default;
		constexpr cMatrix(const cVector3<T>& _x, const cVector3<T>& _y, const cVector3<T>& _z)                        : x( _x, T( 0 ) ), y( _y, T( 0 ) ), z( _z, T( 0 ) ), w( kZero ) {}
		constexpr cMatrix(const cVector3<T>& _x, const cVector3<T>& _y, const cVector3<T>& _z, const cVector3<T>& _w) : x( _x, T( 0 ) ), y( _y, T( 0 ) ), z( _z, T( 0 ) ), w( _w ) {}
		constexpr cMatrix(const cVector4<T>& _x, const cVector4<T>& _y, const cVector4<T>& _z, const cVector4<T>& _w) : x(_x), y(_y), z(_z), w(_w) {}


		// Hardcore but direct
		//constexpr Matrix operator*(const Matrix& _m) const { return { { x.x * _m.x.x + x.y * _m.y.x + x.z * _m.z.x + x.w * _m.w.x, x.x * _m.x.y + x.y * _m.y.y + x.z * _m.z.y + x.w * _m.w.y, x.x * _m.x.z + x.y * _m.y.z + x.z * _m.z.z + x.w * _m.w.z, x.x * _m.x.w + x.y * _m.y.w + x.z * _m.z.w + x.w * _m.w.w, }, { y.x * _m.x.x + y.y * _m.y.x + y.z * _m.z.x + y.w * _m.w.x, y.x * _m.x.y + y.y * _m.y.y + y.z * _m.z.y + y.w * _m.w.y, y.x * _m.x.z + y.y * _m.y.z + y.z * _m.z.z + y.w * _m.w.z, y.x * _m.x.w + y.y * _m.y.w + y.z * _m.z.w + y.w * _m.w.w, }, { z.x * _m.x.x + z.y * _m.y.x + z.z * _m.z.x + z.w * _m.w.x, z.x * _m.x.y + z.y * _m.y.y + z.z * _m.z.y + z.w * _m.w.y, z.x * _m.x.z + z.y * _m.y.z + z.z * _m.z.z + z.w * _m.w.z, z.x * _m.x.w + z.y * _m.y.w + z.z * _m.z.w + z.w * _m.w.w, }, { w.x * _m.x.x + w.y * _m.y.x + w.z * _m.z.x + w.w * _m.w.x, w.x * _m.x.y + w.y * _m.y.y + w.z * _m.z.y + w.w * _m.w.y, w.x * _m.x.z + w.y * _m.y.z + w.z * _m.z.z + w.w * _m.w.z, w.x * _m.x.w + w.y * _m.y.w + w.z * _m.z.w + w.w * _m.w.w, } }; }

		float& operator()( size_t _r, size_t _c )       { return ( *this )[ _r ][ _c ]; }
		float& operator()( size_t _r, size_t _c ) const { return ( *this )[ _r ][ _c ]; }

		cVector4< T >& operator[]( size_t _r )       { return ( &x )[ _r ]; }
		cVector4< T >& operator[]( size_t _r ) const { return ( &x )[ _r ]; }

		constexpr cMatrix operator+(const cMatrix& _m) const { return { _m.x + x, _m.y + y, _m.z + z, _m.w + w }; }
		constexpr cMatrix operator-(const cMatrix& _m) const { return { _m.x - x, _m.y - y, _m.z - z, _m.w - w }; }
		constexpr cMatrix operator*(const cMatrix& _m) const { return { (_m.x * x.x) + (_m.y * x.y) + (_m.z * x.z) + (_m.w * x.w), (_m.x * y.x) + (_m.y * y.y) + (_m.z * y.z) + (_m.w * y.w), (_m.x * z.x) + (_m.y * z.y) + (_m.z * z.z) + (_m.w * z.w), (_m.x * w.x) + (_m.y * w.y) + (_m.z * w.z) + (_m.w * w.w) }; }

		constexpr cMatrix& operator= ( const cMatrix&  _m )          { x = _m.x; y = _m.y; z = _m.z; w = _m.w; return *this; }
		constexpr cMatrix& operator= (       cMatrix&& _m ) noexcept { x = _m.x; y = _m.y; z = _m.z; w = _m.w; return *this; }

		template< class Ty2 >
		constexpr cMatrix& operator= ( const cMatrix4x4< Ty2 >&  _m ){ x = _m.x; y = _m.y; z = _m.z; w = _m.w; return *this; }

		constexpr cMatrix& operator+=(const cMatrix& _m){ x += _m.x; y += _m.y; z += _m.z; w += _m.w; return *this; }
		constexpr cMatrix& operator-=(const cMatrix& _m){ x -= _m.x; y -= _m.y; z -= _m.z; w -= _m.w; return *this; }
		constexpr cMatrix& operator*=(const cMatrix& _m){ x = (_m.x * x.x) + (_m.y * x.y) + (_m.z * x.z) + (_m.w * x.w); y = (_m.x * y.x) + (_m.y * y.y) + (_m.z * y.z) + (_m.w * y.w); z = (_m.x * z.x) + (_m.y * z.y) + (_m.z * z.z) + (_m.w * z.w); w = (_m.x * w.x) + (_m.y * w.y) + (_m.z * w.z) + (_m.w * w.w); return *this; }

		constexpr auto& transpose( void ) const
		{
			transposed( *this );
			return *this;
		} // transpose

		constexpr auto transposed( void ) const
		{
			return cMatrix{ { x.x, y.x, z.x, w.x }, { x.y, y.y, z.y, w.y }, { x.z, y.z, z.z, w.z }, { x.w, y.w, z.w, w.w } };
		} // transpose

		constexpr void transposed( cMatrix& _out ) const
		{
			_out = { { x.x, y.x, z.x, w.x }, { x.y, y.y, z.y, w.y }, { x.z, y.z, z.z, w.z }, { x.w, y.w, z.w, w.w } };
		} // transpose

		auto& transpose_fast( void ) const
		{
			if constexpr( std::is_same_v< T, float > )
				transposed_fast( *this, *this );
			else
			{
				printf( "Warning: Matrix isn't of type float, so doesn't have support for transposed_fast" );
				transposed( *this );
			}
			return *this;
		} // transpose

		auto transposed_fast( void ) const
		{
			cMatrix result;
			if constexpr( std::is_same_v< T, float > )
				transposed_fast( *this, result );
			else
			{
				printf( "Warning: Matrix isn't of type float, so doesn't have support for transposed_fast" );
				transposed( result );
			}
			return result;
		}

		static void transposed_fast( const cMatrix& _in, cMatrix& _out )
		{
			const auto in  = reinterpret_cast< float* >( &_in );
			const auto out = reinterpret_cast< float* >( &_out );

			__m128 row0 = _mm_load_ps( in );
			__m128 row1 = _mm_load_ps( in + 1 );
			__m128 row2 = _mm_load_ps( in + 2 );
			__m128 row3 = _mm_load_ps( in + 3 );
			_MM_TRANSPOSE4_PS( row0, row1, row2, row3 );
			_mm_store_ps( out,     row0 );
			_mm_store_ps( out + 1, row1 );
			_mm_store_ps( out + 2, row2 );
			_mm_store_ps( out + 3, row3 );
		} // transpose

		constexpr auto& inverse( void )
		{
			inversed( *this );
			return *this;
		} // inverse

		constexpr auto inversed( void ) const
		{
			cMatrix out;
			invsered( out );
			return out;
		} // inversed

		constexpr void inversed( cMatrix& _out ) const
		{
			// From: https://stackoverflow.com/a/60374938
			auto& m  = *this;
			auto& im = _out;

		    T A2323 = m(2, 2) * m(3, 3) - m(2, 3) * m(3, 2);
		    T A1323 = m(2, 1) * m(3, 3) - m(2, 3) * m(3, 1);
		    T A1223 = m(2, 1) * m(3, 2) - m(2, 2) * m(3, 1);
		    T A0323 = m(2, 0) * m(3, 3) - m(2, 3) * m(3, 0);
		    T A0223 = m(2, 0) * m(3, 2) - m(2, 2) * m(3, 0);
		    T A0123 = m(2, 0) * m(3, 1) - m(2, 1) * m(3, 0);
		    T A2313 = m(1, 2) * m(3, 3) - m(1, 3) * m(3, 2);
		    T A1313 = m(1, 1) * m(3, 3) - m(1, 3) * m(3, 1);
		    T A1213 = m(1, 1) * m(3, 2) - m(1, 2) * m(3, 1);
		    T A2312 = m(1, 2) * m(2, 3) - m(1, 3) * m(2, 2);
		    T A1312 = m(1, 1) * m(2, 3) - m(1, 3) * m(2, 1);
		    T A1212 = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1);
		    T A0313 = m(1, 0) * m(3, 3) - m(1, 3) * m(3, 0);
		    T A0213 = m(1, 0) * m(3, 2) - m(1, 2) * m(3, 0);
		    T A0312 = m(1, 0) * m(2, 3) - m(1, 3) * m(2, 0);
		    T A0212 = m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0);
		    T A0113 = m(1, 0) * m(3, 1) - m(1, 1) * m(3, 0);
		    T A0112 = m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0);

		    T det = m(0, 0) * ( m(1, 1) * A2323 - m(1, 2) * A1323 + m(1, 3) * A1223 )
		        - m(0, 1) * ( m(1, 0) * A2323 - m(1, 2) * A0323 + m(1, 3) * A0223 )
		        + m(0, 2) * ( m(1, 0) * A1323 - m(1, 1) * A0323 + m(1, 3) * A0123 )
		        - m(0, 3) * ( m(1, 0) * A1223 - m(1, 1) * A0223 + m(1, 2) * A0123 );
		    det = 1 / det;

		    im(0, 0) = det *   ( m(1, 1) * A2323 - m(1, 2) * A1323 + m(1, 3) * A1223 );
		    im(0, 1) = det * - ( m(0, 1) * A2323 - m(0, 2) * A1323 + m(0, 3) * A1223 );
		    im(0, 2) = det *   ( m(0, 1) * A2313 - m(0, 2) * A1313 + m(0, 3) * A1213 );
		    im(0, 3) = det * - ( m(0, 1) * A2312 - m(0, 2) * A1312 + m(0, 3) * A1212 );
		    im(1, 0) = det * - ( m(1, 0) * A2323 - m(1, 2) * A0323 + m(1, 3) * A0223 );
		    im(1, 1) = det *   ( m(0, 0) * A2323 - m(0, 2) * A0323 + m(0, 3) * A0223 );
		    im(1, 2) = det * - ( m(0, 0) * A2313 - m(0, 2) * A0313 + m(0, 3) * A0213 );
		    im(1, 3) = det *   ( m(0, 0) * A2312 - m(0, 2) * A0312 + m(0, 3) * A0212 );
		    im(2, 0) = det *   ( m(1, 0) * A1323 - m(1, 1) * A0323 + m(1, 3) * A0123 );
		    im(2, 1) = det * - ( m(0, 0) * A1323 - m(0, 1) * A0323 + m(0, 3) * A0123 );
		    im(2, 2) = det *   ( m(0, 0) * A1313 - m(0, 1) * A0313 + m(0, 3) * A0113 );
		    im(2, 3) = det * - ( m(0, 0) * A1312 - m(0, 1) * A0312 + m(0, 3) * A0112 );
		    im(3, 0) = det * - ( m(1, 0) * A1223 - m(1, 1) * A0223 + m(1, 2) * A0123 );
		    im(3, 1) = det *   ( m(0, 0) * A1223 - m(0, 1) * A0223 + m(0, 2) * A0123 );
		    im(3, 2) = det * - ( m(0, 0) * A1213 - m(0, 1) * A0213 + m(0, 2) * A0113 );
		    im(3, 3) = det *   ( m(0, 0) * A1212 - m(0, 1) * A0212 + m(0, 2) * A0112 );
		} // inverse_safe

		auto& inverse_fast( void )
		{
			if constexpr( std::is_same_v< T, float > )
				inversed_fast( *this, *this );
			else
			{
				printf( "Warning: Matrix isn't of type float, so doesn't have support for inverse_fast" );
				inversed( *this );
			}
			return *this;
		} // inverse_fast

		auto inversed_fast( void )
		{
			cMatrix4x4f out;
			if constexpr( std::is_same_v< T, float > )
				inversed_fast( *this, out );
			else
			{
				printf( "Warning: Matrix isn't of type float, so doesn't have support for inverse_fast" );
				inversed( out );
			}

			return out;
		} // inversed_fast

		void inversed_fast( cMatrix4x4f& _out ) const
		{
			if constexpr( std::is_same_v< T, float > )
				inversed_fast( *this, _out );
			else
			{
				const cMatrix4x4f in = *this;
				inversed_fast( in, _out );
			}
		} // inversed_fast

		static void inversed_fast( const cMatrix4x4f& _in, cMatrix4x4f& _out )
		{
			const auto as_m  = reinterpret_cast< const __m64* >( &_in );
			__m64*     out_m = reinterpret_cast<       __m64* >( &_out );

			__m128 row1{}, row3{};
			__m128 tmp1{};

			tmp1 = _mm_loadh_pi( _mm_loadl_pi( tmp1, as_m     ), as_m + 2 );
			row1 = _mm_loadh_pi( _mm_loadl_pi( row1, as_m + 4 ), as_m + 6 );

			const auto row0 = _mm_shuffle_ps( tmp1 , row1 , 0x88 );
			row1 = _mm_shuffle_ps( row1, tmp1, 0xDD );

			tmp1 = _mm_loadh_pi( _mm_loadl_pi( tmp1, as_m + 1 ), as_m + 3 );
			row3 = _mm_loadh_pi( _mm_loadl_pi( row3, as_m + 5 ), as_m + 7 );

			auto row2 = _mm_shuffle_ps( tmp1 , row3 , 0x88 );
			row3 = _mm_shuffle_ps( row3, tmp1, 0xDD ); // 11


			tmp1 = _mm_mul_ps( row2, row3 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );

			__m128 minor0 = _mm_mul_ps( row1 , tmp1 );
			__m128 minor1 = _mm_mul_ps( row0 , tmp1 );

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor0 = _mm_sub_ps( _mm_mul_ps( row1, tmp1 ), minor0 );
			minor1 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor1 );
			minor1 = _mm_shuffle_ps( minor1, minor1, 0x4E ); // 19


			tmp1 = _mm_mul_ps( row1, row2 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );

			minor0 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor0 );
			__m128 minor3 = _mm_mul_ps( row0 , tmp1 ); // 23

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row3, tmp1 ) );
			minor3 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor3 );
			minor3 = _mm_shuffle_ps( minor3, minor3, 0x4E ); // 27


			tmp1 = _mm_mul_ps( _mm_shuffle_ps( row1, row1, 0x4E ), row3 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
			row2 = _mm_shuffle_ps( row2, row2, 0x4E );

			minor0 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor0 );
			__m128 minor2 = _mm_mul_ps( row2 , tmp1 );

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row2, tmp1 ) );
			minor2 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor2 );
			minor2 = _mm_shuffle_ps( minor2, minor2, 0x4E ); // 36


			tmp1 = _mm_mul_ps( row0, row1 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );

			minor2 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
			minor3 = _mm_sub_ps( _mm_mul_ps( row2, tmp1 ), minor3 );

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor2 = _mm_sub_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
			minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row2, tmp1 ) ); // 43

			tmp1 = _mm_mul_ps( row0, row3 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );

			minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row2, tmp1 ) );
			minor2 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor2 );

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor1 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor1 );
			minor2 = _mm_sub_ps( minor2, _mm_mul_ps( row1, tmp1 ) ); // 50


			tmp1 = _mm_mul_ps( row0, row2 );
			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );

			minor1 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor1 );
			minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row1, tmp1 ) );

			tmp1 = _mm_shuffle_ps( tmp1, tmp1, 0x4E );

			minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row3, tmp1 ) );
			minor3 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor3 ); // 57


			__m128 det = _mm_mul_ps( row0 , minor0 );
			det = _mm_add_ps( _mm_shuffle_ps( det, det, 0x4E ), det );
			det = _mm_add_ss( _mm_shuffle_ps( det, det, 0xB1 ), det );
			tmp1 = _mm_rcp_ss( det );

			det = _mm_sub_ss( _mm_add_ss( tmp1, tmp1 ), _mm_mul_ss( det, _mm_mul_ss( tmp1, tmp1 ) ) );
			det = _mm_shuffle_ps( det, det, 0x00 );

			minor0 = _mm_mul_ps( det, minor0 );
			_mm_storel_pi( out_m,     minor0 );
			_mm_storeh_pi( out_m + 1, minor0 );

			minor1 = _mm_mul_ps( det, minor1 );
			_mm_storel_pi( out_m + 2, minor1 );
			_mm_storeh_pi( out_m + 3, minor1 );

			minor2 = _mm_mul_ps( det, minor2 );
			_mm_storel_pi( out_m + 4, minor2 );
			_mm_storeh_pi( out_m + 5, minor2 );

			minor3 = _mm_mul_ps( det, minor3 );
			_mm_storel_pi( out_m + 6, minor3 );
			_mm_storeh_pi( out_m + 7, minor3 );
		} // inverse_fast
	};

	namespace Matrix4x4
	{
		template <typename T> constexpr cMatrix4x4<T> rotateX(const float& _a)
		{
			return {
				qw::cVector3< T >{ T(1), T(0),	T(0), T(0)},
				qw::cVector3< T >{ T(0), Math::cos(_a), -Math::sin(_a), T(0)},
				qw::cVector3< T >{ T(0), Math::sin(_a), Math::cos(_a), T(0) },
			};
		}
		template <typename T> constexpr cMatrix4x4<T> rotateY(const float& _a)
		{
			return {
				qw::cVector3< T >{ Math::cos(_a), T(0), -Math::sin(_a), T(0) },
				qw::cVector3< T >{ T(0), T(1), T(0), T(0) },
				qw::cVector3< T >{ Math::sin(_a), T(0), Math::cos(_a), T(0) },
			};
		}
		template <typename T> constexpr cMatrix4x4<T> rotateZ(const float& _a)
		{
			return {
				qw::cVector3< T >{ Math::cos(_a), -Math::sin(_a),T(0), T(0)},
				qw::cVector3< T >{ Math::sin(_a), Math::cos(_a), T(0), T(0) },
				qw::cVector3< T >{ T(0), T(0), T(1), T(0) },
			};
		}
		// Rotates around XYZ according to euler angles
		template <typename T> constexpr cMatrix4x4<T> rotate(const cVector3<T>& _v)
		{
			return {
				qw::cVector3< T >{
					Math::cos(_v.y) * Math::cos(_v.z),
					Math::sin(_v.x) * Math::sin(_v.y) * Math::cos(_v.z) - Math::cos(_v.x) * Math::sin(_v.z),
					Math::cos(_v.x) * Math::sin(_v.y) * Math::cos(_v.z) + Math::sin(_v.x) * Math::sin(_v.z),
				},
				qw::cVector3< T >{
					Math::cos(_v.y) * Math::sin(_v.z),
					Math::sin(_v.x) * Math::sin(_v.y) * Math::sin(_v.z) + Math::cos(_v.x) * Math::cos(_v.z),
					Math::cos(_v.x) * Math::sin(_v.y) * Math::sin(_v.z) - Math::sin(_v.x) * Math::cos(_v.z),
				},
				qw::cVector3< T >{
					-Math::sin(_v.y),
					Math::sin(_v.x) * Math::cos(_v.y),
					Math::cos(_v.x) * Math::cos(_v.y),
				},
			};
		}
		// Rotates around axis
		template <typename T> cMatrix4x4< T >rotate(const cVector3<T>& _v, const float& _a)
		{

			const cVector3< T > _n = Vector3::Normalized(_v);
			return {
				qw::cVector3< T >{
					_v.x * _v.x * (1 - Math::cos(_a)) + Math::cos(_a),
					_v.x * _v.y * (1 - Math::cos(_a)) - _v.z * Math::sin(_a),
					_v.x * _v.z * (1 - Math::cos(_a)) + _v.y * Math::sin(_a), T(0)
				},
				qw::cVector3< T >{
					_v.x * _v.y * (1 - Math::cos(_a)) + _v.z * Math::sin(_a),
					_v.y * _v.y * (1 - Math::cos(_a)) + Math::cos(_a),
					_v.y * _v.z * (1 - Math::cos(_a)) - _v.x * Math::sin(_a), T(0)
				},
				qw::cVector3< T >{
					_v.x * _v.z * (1 - Math::cos(_a)) - _v.y * Math::sin(_a),
					_v.y * _v.z * (1 - Math::cos(_a)) + _v.x * Math::sin(_a),
					_v.z * _v.z * (1 - Math::cos(_a)) + Math::cos(_a), T(0)
				},
			};
		}

		template< class T >
		constexpr cMatrix4x4< T > scale_rotate_translate( const cVector3< T >& _scale, const cVector3< T >& _rotation, const cVector3< T >& _location )
		{
			const auto& sx = _scale.x;
			const auto& sy = _scale.y;
			const auto& sz = _scale.z;

			// TODO: Maybe change how vector is interpreted as rotation?

			// Source: https://en.wikipedia.org/wiki/Rotation_matrix#General_3D_rotations
			// New? https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix

			const auto& rx = _rotation.x;
			const auto& ry = _rotation.y;
			const auto& rz = _rotation.z;

			const auto srx = Math::sin(rx);
			const auto sry = Math::sin(ry);
			const auto srz = Math::sin(rz);

			const auto crx = Math::cos(rx);
			const auto cry = Math::cos(ry);
			const auto crz = Math::cos(rz);

			return cMatrix4x4< T >(
				qw::cVector3< T >{
					cry * crz,
				   -cry * srz,
					sry,
				} * sx,
				qw::cVector3< T >{
					crx * srz + crz * srx * sry,
					crx * crz - srx * sry * srz,
				   -cry * srx,
				} * sy,
				qw::cVector3< T >{
					srx * srz - crx * crz * sry,
					crz * srx + crx * sry * srz,
					crx * cry,
				} * sz,
				_location
			);
		}

	} // Matrix4x4::
} // qw::