/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Math.h"

namespace qw
{
	/// <summary>
	/// Just don't do this...
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <typeparam name="S"></typeparam>
	template< size_t S, typename T > class cVector
	{
	public:
		cVector() = default;

		T _vector[ S ]{};
		size_t size = S;

		constexpr T& operator[](const size_t _i) { return _vector[_i]; }
		constexpr const T& operator[](const size_t _i) const { return _vector[_i]; }

	}; // cVector

} // qw::