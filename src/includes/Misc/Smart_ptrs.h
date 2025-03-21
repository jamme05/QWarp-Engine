/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <atomic>
#include <memory>

#include "Memory/Tracker/cTracker.h"

namespace qw
{
	namespace Ptr_logic
	{
		class cData_base
		{
			virtual void  deleteSelf( void ) = 0;
			virtual void  deleteCont( void ) = 0;
		protected:
			cData_base( void )
			: m_ref_count( 1 )
			, m_weak_ref_count( 0 )
			{}

			virtual ~cData_base( void ) = default;

		public:
			virtual void* get_ptr   ( void ) = 0;

			void inc( void )
			{
				++m_ref_count;
			}

			void inc_weak( void )
			{
				++m_weak_ref_count;
			}

			void dec( void )
			{
				if( --m_ref_count == 0 )
				{
					deleteCont();
					if( !m_weak_ref_count )
						deleteSelf();
				}
			}

			void dec_weak( void )
			{
				if( --m_weak_ref_count == 0 && !m_ref_count )
				{
					deleteSelf();
				}
			}
		private:
			std::atomic_uint32_t m_ref_count;
			// In case weak_ptr ever happens
			std::atomic_uint32_t m_weak_ref_count;
		};

		template< class Ty >
		class cData : public cData_base
		{
			void deleteCont( void ) override
			{
				QW_FREE( m_ptr );
			}
			void deleteSelf( void ) override
			{
				QW_FREE( this );
			}
		public:
			void* get_ptr( void ) override { return m_ptr; }

			explicit cData( Ty* _content )
			: m_ptr( _content )
			{}

			~cData( void ) override = default;

		protected:
			Ty* m_ptr;
		};
	} // Ptr_logic::

	class cPtr_base
	{
	public:
		auto& operator=( const cPtr_base& _other )
		{
			if( this != &_other )
				m_data = _other.m_data;

			return *this;
		}

	protected:
		void inc     ( void ) const { if( m_data ) m_data->inc(); }
		void dec     ( void ) const { if( m_data ) m_data->dec(); }
		void inc_weak( void ) const { if( m_data ) m_data->inc_weak(); }
		void dec_weak( void ) const { if( m_data ) m_data->dec_weak(); }

		Ptr_logic::cData_base* m_data = nullptr;

		template< class Fy >
		friend class cShared_ptr;
	};

	// Shared ptr using itself of the tracker. Use in case the class is humongous.
	template< class Ty >
	class cShared_ptr : public cPtr_base
	{
		template< class Fy >
		friend class cShared_ptr;

		Ty* m_ptr  = nullptr;

		public:
		cShared_ptr( void )
		{
		} // cShared_ptr

		explicit cShared_ptr( Ty* _ptr )
		{
			m_data = QW_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			m_ptr  = _ptr;
		} // cShared_ptr

		cShared_ptr( nullptr_t )
		{
			m_data = nullptr;
			m_ptr  = nullptr;
		} // cShared_ptr

		cShared_ptr( const cShared_ptr& _right )
		{
			m_data = _right.m_data;
			m_ptr  = _right.m_ptr;
			inc();

		} // cShared_ptr

		cShared_ptr( cShared_ptr&& _right ) noexcept
		{
			m_data = _right.m_data;
			m_ptr  = _right.m_ptr;
			_right.m_data = nullptr;
			_right.m_ptr  = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cShared_ptr( const cShared_ptr< Other >& _right )
		{
			m_data = _right.m_data;
			m_ptr  = static_cast< Ty* >( _right.m_ptr );
			inc();
		} // cShared_ptr

		cShared_ptr( const cPtr_base& _right )
		{
			m_data = _right.m_data;

			if( m_data )
				m_ptr = static_cast< Ty* >( m_data->get_ptr() );
		} // cShared_ptr

		~cShared_ptr( void )
		{
			dec();
		} // ~cShared_ptr

		auto& operator=( const cShared_ptr& _right )
		{
			if( m_data != _right.m_data && this != &_right )
			{
				dec();
				m_data = _right.m_data;
				m_ptr  = _right.m_ptr;
				inc();
			}

			return *this;
		}

		auto& operator=( cShared_ptr&& _right ) noexcept
		{
			dec();
			m_data = _right.m_data;
			m_ptr  = _right.m_ptr;
			_right.m_data = nullptr;
			_right.m_ptr  = nullptr;

			return *this;
		}

		auto& operator=( std::nullptr_t )
		{
			dec();
			m_data = nullptr;
			m_ptr  = nullptr;

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_convertible_v< Other, Ty >, cShared_ptr& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			m_ptr  = static_cast< Ty* >( _right.m_ptr );
			inc();

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other, Ty >, cShared_ptr& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			m_ptr  = static_cast< Ty* >( _right.m_ptr );
			inc();

			return *this;
		}

		operator bool( void ) const { return ( m_data != nullptr ); }

		Ty& operator  *( void ){ return *m_ptr; }
		Ty* operator ->( void ){ return  m_ptr; }

		Ty& operator  *( void ) const { return *m_ptr; }
		Ty* operator ->( void ) const { return  m_ptr; }

		operator Ty*   ( void )       { return m_ptr; }
		operator Ty*   ( void ) const { return m_ptr; }

		Ty* get( void )       { return m_ptr; }
		Ty* get( void ) const { return m_ptr; }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		std::enable_if_t< std::is_base_of_v< Ty, Ot > || std::is_base_of_v< Ot, Ty >,
		cShared_ptr< Ot > > cast( void )
		{
			Ot* op = static_cast< Ot* >( m_ptr );

			cShared_ptr< Ot > other{};
			other.m_data = m_data;
			other.m_ptr  = op;

			return other;
		}
	};

	template< class Ty >
	class sShared_from_this
	{
	protected:
		auto get_shared_this( void )       { return cShared_ptr< Ty >( static_cast< Ty* >( this ) ); }
		auto get_shared_this( void ) const { return cShared_ptr< Ty >( static_cast< Ty* >( this ) ); }
	};

	template< class Ty, class... Args >
	auto make_shared( Args... _args ) -> cShared_ptr< Ty >
	{
		Ty* ptr = QW_NEW( Ty, 1, _args... );

		return cShared_ptr< Ty >( ptr );
	}
} // qw::