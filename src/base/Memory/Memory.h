/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#if !defined( QW_CORE_ID )
#define QW_CORE_ID (-1)
#endif

#if QW_CORE_ID == -1
#define OVERRIDE_TRACKER
#else // QW_CORE_ID == -1 aka invalid core.
#include "Memory/Tracker/cTracker.h"
#endif // !QW_NO_CORE

namespace qw::Memory
{
    template< class Ty >
    constexpr size_t get_size( const size_t _count ) noexcept { return sizeof( Ty ) * _count; }
} // qw::Memory

#if defined( OVERRIDE_TRACKER )
#define QW_ALLOC( Size ) ::malloc( Size )
#define QW_FREE( Ptr ) ::free( Ptr )
#define MAX_HEAP_SIZE std::numeric_limits< size_t >::max()
#endif // OVERRIDE_TRACKER