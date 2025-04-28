/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <memory>

#include "Memory/Tracker/cTracker.h"

namespace qw
{
	template< class Ty >
	class allocator
	{
	public:
		typedef Ty             value_type;
		typedef Ty*            pointer;
		typedef const Ty*      const_pointer;
		typedef Ty&            reference;
		typedef const Ty&      const_reference;
		typedef size_t         size_type;
		typedef ptrdiff_t      difference_type;
		typedef std::true_type propagate_on_container_move_assignment;
		template< class Ot >
		struct rebind
		{
			typedef allocator< Ot > other;
		};
		typedef std::true_type is_always_equal;

	public:
		// TODO: Constructors
		allocator( void ) noexcept = default;
		allocator( const allocator& ) noexcept = default;
		allocator( allocator&& ) noexcept = default;
		template< class Ty2 >
		allocator( const allocator< Ty2 >& ) noexcept { }
		~allocator( void ) noexcept = default;

		auto& operator=( const allocator& ) noexcept { return *this; };
		auto& operator=( allocator&& ) noexcept { return *this; };
		template< class Ty2 >
		auto& operator=( const allocator< Ty2 >& ) noexcept { return *this; }
		template< class Ty2 >
		auto& operator=( allocator< Ty2 >&& ) noexcept { return *this; }

		Ty* allocate( const size_type _n )
		{
			if( _n == 0 )
				return nullptr;

			auto byte_size = Memory::get_size< Ty >( _n );

			return static_cast< Ty* >( QW_ALLOC( byte_size ) );
		} // allocate

		void deallocate( void* _p, size_t _n )
		{
			QW_FREE( static_cast< void* >( _p ) );
		} // deallocate

		size_type max_size( void ) const
		{
			return Memory::Tracker::max_heap_size() / sizeof( Ty );
		} // max_size

	};
} // qw::

template<class T, class U>
bool operator==(const qw::allocator<T>&, const qw::allocator <U>&) { return true; }
template<class T, class U>
bool operator!=(const qw::allocator <T>&, const qw::allocator <U>&) { return false; }
