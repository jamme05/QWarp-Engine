/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Memory/cInternal.h"
#include "Misc/cSingleton.h"

#include <unordered_map>
#include <mutex>

namespace qw::Memory
{
	namespace Tracker
	{
		struct sMem_header
		{
			size_t item_size;
			size_t item_count;
		};

		// Can track source usage
		extern void* alloc( const char* _file_name, const char* _function, const uint32_t _line, size_t _size );
		extern void  add  ( const char* _file_name, const char* _function, const uint32_t _line, void*  _block );
		extern void  free ( void* _block );
		extern size_t max_heap_size( void );

	} // Tracker::

	class cTracker : public cSingleton< cTracker >
	{
		struct sTracker_entry
		{
			const char* file_name;
			const char* function;
			uint32_t    line;
			size_t      total_size;
			uint32_t    flags;
		};

		struct sStatistics
		{
			size_t memory_allocated = 0u;
			size_t memory_freed     = 0u;
			size_t allocation_count = 0u;
			size_t free_count       = 0u;
		};

		typedef std::unordered_map< void*, sTracker_entry > block_map_t;

		block_map_t m_block_map;
		sStatistics m_statistics;
		std::mutex  m_mtx;


	public:
		void* alloc( const char* _file_name, const char* _function, const uint32_t _line, const size_t _size );
		void  free ( void* _block );
		void  free ( const block_map_t::value_type& _block );
		void  add  ( const char* _file_name, const char* _function, const uint32_t _line, void* _block );
		// TODO: Add realloc

		 cTracker( void );
		~cTracker( void );

		static void* Alloc( const char* _file_name, const char* _function, const uint32_t _line, const size_t _size ){ return get().alloc( _file_name, _function, _line, _size ); }
		static void  Add  ( const char* _file_name, const char* _function, const uint32_t _line, void*  _block ){ get().add( _file_name, _function, _line, _block ); }
		static void  Free ( void*  _block ){ get().free( _block ); }

		static size_t GetAvailableHeapSpace( void );

		static size_t m_memory_usage;

	}; // cTracker

	extern void* alloc_fast( size_t _size );
	extern void  free_fast ( void*  _block );

	template< typename Ty, typename... Args >
	static Ty* alloc( const char* _file_name, const char* _function, const uint32_t _line, const size_t _count, Args&&... _args )
	{
		const size_t size = get_size< Ty >( _count ) + sizeof( size_t );

		auto count_ptr = static_cast< size_t* >( Tracker::alloc( _file_name, _function, _line, size ) );
		    *count_ptr = _count;

		Ty* ptr = reinterpret_cast< Ty* >( count_ptr + 1 );

		for( size_t i = 0; i < _count; i++ )
			::new( ptr + i ) Ty( std::forward< Args >( _args )... );

		return ptr;

	} // alloc

	template< typename Ty, typename... Args >
	static Ty* alloc_no_tracker( const size_t _count, Args&&... _args )
	{
		const size_t size = get_size< Ty >( _count ) + sizeof( size_t );

		auto count_ptr = static_cast< size_t* >( Memory::alloc_fast( size ) );
		*count_ptr = _count;

		Ty* ptr = reinterpret_cast< Ty* >( count_ptr + 1 );

		for( size_t i = 0; i < _count; i++ )
			::new( ptr + i ) Ty( std::forward< Args >( _args )... );

		return ptr;

	} // alloc_no_tracker

	template< typename Ty >
	static void free( Ty* _block )
	{
		const auto count_ptr = reinterpret_cast< size_t* >( _block ) - 1;

		for( size_t i = 0; i < *count_ptr; i++ )
			_block[ i ].~Ty();

#if defined( QW_TRACKER_DISABLED )
		Memory::free_fast( count_ptr );
#else  // DEBUG
		Tracker::free( count_ptr );
#endif // !DEBUG

	} // free

	extern void free( void* _block );

} // qw::Memory::

#if !defined( QW_TRACKER_DISABLED )
/**
 * Default tracked alloc.
 * 
 * Arguments: Byte Size
 */
#define QW_ALLOC( Size ) qw::Memory::Tracker::alloc( __FILE__, __FUNCTION__, __LINE__, Size )
/**
 * Default tracked new.
 * 
 * Arguments: Type, Count, Args...
 */
#define QW_NEW( Ty, ... ) qw::Memory::alloc< Ty >( __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
/**
 * Virtual array tracked new.
 *
 * Makes a pointer array with declared type.
 * 
 * Arguments: Type, Count.
 */
#define QW_VIRTUAL( Ty, Count ) qw::Memory::alloc< Ty* >( __FILE__, __FUNCTION__, __LINE__, Count, nullptr )
/**
 * Single object tracked new.
 * 
 * Arguments: Type, Args...
 */
#define QW_SINGLE( Ty, ... ) qw::Memory::alloc< Ty >( __FILE__, __FUNCTION__, __LINE__, 1, __VA_ARGS__ )
/**
 * Single object without params tracked new.
 * 
 * Arguments: Type, Args...
 */
#define QW_SINGLE_EMPTY( Ty ) qw::Memory::alloc< Ty >( __FILE__, __FUNCTION__, __LINE__, 1 )
/**
 * Tracked free.
 * 
 * Arguments: Address
 */
#define QW_FREE( address ) qw::Memory::free( address )
#else // !QW_TRACKER_DISABLED
/**
 * Default tracked alloc.
 * 
 * Arguments: Byte Size
 */
#define QW_ALLOC( Size ) qw::Memory::alloc_fast( Size )
/**
 * Default new.
 * 
 * Arguments: Type, Count, Args...
 */
#define QW_NEW( Ty, ... ) qw::Memory::alloc_no_tracker< Ty >( __VA_ARGS__ )
/**
 * Virtual array tracked new.
 *
 * Makes a pointer array with declared type.
 * 
 * Arguments: Type, Count.
 */
#define QW_VIRTUAL( Ty, Count ) qw::Memory::alloc_no_tracker< Ty* >( Count, nullptr )
/**
 * Single object tracked new.
 * 
 * Arguments: Type, Args...
 */
#define QW_SINGLE( Ty, ... ) qw::Memory::alloc_no_tracker< Ty >( 1, __VA_ARGS__ )
/**
 * Single object without params tracked new.
 * 
 * Arguments: Type, Args...
 */
#define QW_SINGLE_EMPTY( Ty ) qw::Memory::alloc_no_tracker< Ty >( 1 )
/**
 * Free.
 * 
 * Arguments: Address
 */
#define QW_FREE( address ) qw::Memory::free( address )


#endif // QW_TRACKER_DISABLED