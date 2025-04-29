/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <math.h>
#include <valarray>

namespace qw
{
	template< class T >
	using enable_arithmetic_t = std::enable_if_t< std::is_arithmetic_v< T >, T >;
	template< class Ty, class Ty2 = Ty >
	concept enable_artimetic_c = std::is_arithmetic_v< Ty > && std::is_arithmetic_v< Ty2 >;
	// TODO: Find if there's a better way, like having two concepts with the same name.

	template< class T >
	struct Promote_to_float
	{
		typedef std::conditional_t< std::is_same_v< T, long double >, long double,
		std::conditional_t< std::is_same_v< T, double > || std::is_integral_v< T >, double, float > > type;
	};

	template< class T >
	using promote_to_float_t = typename Promote_to_float< T >::type;

	template< class T1, class T2 >
	struct Common_float_type
	{
		typedef promote_to_float_t< T1 > Ty1;
		typedef promote_to_float_t< T2 > Ty2;

		typedef std::conditional_t< std::is_same_v< Ty1, long double > || std::is_same_v< Ty2, long double >, long double,
		        std::conditional_t< std::is_same_v< Ty1, double >      || std::is_same_v< Ty2, double >, double, float > > type;
	};

	template< class T1, class T2 >
	using common_float_type_t = typename Common_float_type< T1, T2 >::type;

	namespace Math
	{
		/*
		 * Function format:
		 *
		 * Code has to be on a single line unless using template or close/above to 150 characters long.
		 *
		 * template< TN... > [ T or return value ]
		 *    [ function_name ]( Args... ){ return [ optional: static_cast< T > ][ code ] }
		 *
		 * Code allowed to be multiline in case of typedef or single line too long.
		 */

		template< class >
		struct sPi
		{};

		template<>
		struct sPi< float >
		{
			constexpr static auto kPi  = 3.14159265358979323846f;
			constexpr static auto kPi2 = kPi * kPi;
			constexpr static auto kDegToRad = kPi / 180.0f;
			constexpr static auto kRadToDeg = 180.0f / kPi;
		};

		template<>
		struct sPi< double >
		{
			constexpr static auto kPi  = 3.14159265358979323846;
			constexpr static auto kPi2 = kPi * kPi;
			constexpr static auto kDegToRad = kPi / 180.0;
			constexpr static auto kRadToDeg = 180.0 / kPi;
		};

		template< class Ty >
		constexpr auto kPi = sPi< Ty >::kPi;
		template< class Ty >
		constexpr auto kPi2 = sPi< Ty >::kPi2;
		template< class Ty >
		constexpr auto kDegToRad = sPi< Ty >::kDegToRad;
		template< class Ty >
		constexpr auto kRadToDeg = sPi< Ty >::kRadToDeg;

		// Math:
		template< class T > T
			pow2( const T _val ){ return _val * _val; }

		template< class T > T
			pow3( const T _val ){ return _val * _val * _val; }

		template< class T1, class T2 >
		requires enable_artimetic_c< T1, T2 >
		T1	pow ( const T1 _val, const T2 _exp )
			{
				typedef common_float_type_t< T1, T2 > type;
				return static_cast< T1 >( ::pow( type( _val ), type( _exp ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	cos ( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::cos( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	sin ( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::sin( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	tan ( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::tan( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	acos( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::cos( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	asin( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::sin( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	atan( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::tan( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		T	sqrt( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::sqrt( type( _val ) ) );
			}

		// Logical
		template< class T >
		requires enable_artimetic_c< T >
		T	abs ( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( ::abs( type( _val ) ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		constexpr T	degToRad( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( kDegToRad< type > * type( _val ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		constexpr T	radToDeg( const T _val )
			{
				typedef promote_to_float_t< T > type;
				return static_cast< T >( kRadToDeg< type > * type( _val ) );
			}

		template< class T >
		requires enable_artimetic_c< T >
		constexpr T	min( const T _a, const T _b )
			{
				return _a < _b ? _a : _b;
			}

		template< class T >
		requires enable_artimetic_c< T >
		constexpr T	max( const T _a, const T _b )
			{
				return _a > _b ? _a : _b;
			}

	} // Math::
} // qw::