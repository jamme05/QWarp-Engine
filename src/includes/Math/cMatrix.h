/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */


#pragma once

#include "cVector.h"
#include "cVector2.h"

namespace qw
{
	template < size_t W, size_t H, typename T > class cMatrix
	{
		cVector< W, T > m_matrix[ H ];

		cVector< W, T >& operator[]( size_t _r ){ return m_matrix[ _r ]; }
	};
} // qw::