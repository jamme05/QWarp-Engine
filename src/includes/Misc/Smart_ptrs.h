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
			: m_ref_count( 0 )
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
				if( --m_ref_count <= 0 && !m_is_deleting ) // TODO: Not use the bool and only change the ref count after deletion?
				{
					m_is_deleting = true;
					deleteCont();
					if( m_weak_ref_count <= 0 )
						deleteSelf();
					m_is_deleting = false;
				}
			}

			void dec_weak( void )
			{
				if( --m_weak_ref_count <= 0 && m_ref_count <= 0 && !m_is_deleting )
				{
					deleteSelf();
				}
			}
		private:
			std::atomic_uint32_t m_ref_count;
			// In case weak_ptr ever happens
			std::atomic_uint32_t m_weak_ref_count;
			std::atomic_bool     m_is_deleting = false;
		};

		template< class Ty >
		class cData : public cData_base
		{
			void deleteCont( void ) override
			{
				QW_FREE( m_ptr );
				m_ptr = nullptr;
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
		cPtr_base( void ) = default;

		cPtr_base( const cPtr_base& _other )
		{
			if( this != &_other )
				m_data = _other.m_data;
		} // cPtr_base

		explicit cPtr_base( Ptr_logic::cData_base* _data )
		{
			m_data = _data;
		} // cPtr_base

		auto& operator=( const cPtr_base& _right )
		{
			if( this != &_right )
				m_data = _right.m_data;

			return *this;
		}

		operator bool ( void ) const { return m_data != nullptr; }

		bool operator!=( const void*      _other ) const { return m_data->get_ptr() != _other; }
		bool operator==( const void*      _other ) const { return m_data->get_ptr() == _other; }
		bool operator!=( const cPtr_base& _other ) const { return m_data != _other.m_data; }
		bool operator==( const cPtr_base& _other ) const { return m_data == _other.m_data; }

	protected:
		void inc     ( void ) const { if( m_data ) m_data->inc(); }
		void dec     ( void ) const { if( m_data ) m_data->dec(); }
		void inc_weak( void ) const { if( m_data ) m_data->inc_weak(); }
		void dec_weak( void ) const { if( m_data ) m_data->dec_weak(); }

		Ptr_logic::cData_base* m_data = nullptr;

		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cShared_Ref;
		template< class Fy >
		friend class cWeak_Ptr;
	};

	template< class Ty >
	class cShared_from_this;

	// Shared ptr using itself of the tracker. Use in case the class is humongous.
	template< class Ty >
	class cShared_ptr : public cPtr_base
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cWeak_Ptr;

		public:
		cShared_ptr( void )
		{
		} // cShared_ptr

		explicit cShared_ptr( Ty* _ptr )
		{
			m_data = QW_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			inc();
		} // cShared_ptr

		cShared_ptr( nullptr_t )
		{
			m_data = nullptr;
		} // cShared_ptr

		cShared_ptr( const cShared_ptr& _right )
		{
			m_data = _right.m_data;
			inc();

		} // cShared_ptr

		cShared_ptr( cShared_ptr&& _right ) noexcept
		{
			m_data = _right.m_data;
			_right.m_data = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cShared_ptr( const cShared_ptr< Other >& _right )
		{
			m_data = _right.m_data;
			inc();
		} // cShared_ptr

		cShared_ptr( const cPtr_base& _right )
		{
			m_data = _right.m_data;

			if( m_data )
			{
				inc();
			}
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
				inc();
			}

			return *this;
		}

		auto& operator=( cShared_ptr&& _right ) noexcept
		{
			if( m_data != _right.m_data )
			{
				dec();
				m_data = _right.m_data;
			}
			_right.m_data = nullptr;

			return *this;
		}

		auto& operator=( std::nullptr_t )
		{
			dec();
			m_data = nullptr;

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_convertible_v< Other, Ty >, cShared_ptr& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			inc();

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other, Ty >, cShared_ptr& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			inc();

			return *this;
		}

		operator bool( void ) const { return ( m_data != nullptr ); }

		Ty& operator  *( void ){ return *get(); }
		Ty* operator ->( void ){ return  get(); }

		Ty& operator  *( void ) const { return *get(); }
		Ty* operator ->( void ) const { return  get(); }

		operator Ty*   ( void )       { return get(); }
		operator Ty*   ( void ) const { return get(); }

		Ty* get( void )       { return static_cast< Ty* >( m_data ? m_data->get_ptr() : nullptr ); }
		Ty* get( void ) const { return static_cast< Ty* >( m_data ? m_data->get_ptr() : nullptr ); }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		std::enable_if_t< std::is_base_of_v< Ty, Ot > || std::is_base_of_v< Ot, Ty >,
		cShared_ptr< Ot > > cast( void )
		{
			cShared_ptr< Ot > other{};
			other.m_data = m_data;
			inc();

			return other;
		}
	};

	template< class Ty >
	class cShared_Ref : public cPtr_base // TODO: add usages.
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cShared_Ref;
		template< class Fy >
		friend class cWeak_Ptr;

		public:
		cShared_Ref( void )
		{
		} // cShared_ptr

		explicit cShared_Ref( Ty* _ptr )
		{
			m_data = QW_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			inc();
		} // cShared_ptr

		cShared_Ref( const cShared_Ref& _right )
		{
			m_data = _right.m_data;
			inc();

		} // cShared_ptr

		cShared_Ref( cShared_Ref&& _right ) noexcept
		{
			m_data = _right.m_data;
			_right.m_data = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cShared_Ref( const cShared_ptr< Other >& _right )
		{
			m_data = _right.m_data;
			inc();
		} // cShared_ptr

		cShared_Ref( const cPtr_base& _right )
		{
			m_data = _right.m_data;

			if( m_data )
			{
				inc();
			}
		} // cShared_ptr

		~cShared_Ref( void )
		{
			dec();
		} // ~cShared_ptr

		auto& operator=( const cShared_Ref& _right )
		{
			if( m_data != _right.m_data && this != &_right )
			{
				dec();
				m_data = _right.m_data;
				inc();
			}

			return *this;
		}

		auto& operator=( cShared_Ref&& _right ) noexcept
		{
			if( m_data != _right.m_data )
			{
				dec();
				m_data = _right.m_data;
			}
			_right.m_data = nullptr;

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_convertible_v< Other, Ty >, cShared_Ref& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			inc();

			return *this;
		}

		template< class Other >
		std::enable_if_t< std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other, Ty >, cShared_Ref& >
		operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data = _right.m_data;
			inc();

			return *this;
		}

		operator bool( void ) const { return ( m_data != nullptr ); }

		Ty& operator  *( void ){ return *get(); }
		Ty* operator ->( void ){ return  get(); }

		Ty& operator  *( void ) const { return *get(); }
		Ty* operator ->( void ) const { return  get(); }

		operator Ty*   ( void )       { return get(); }
		operator Ty*   ( void ) const { return get(); }

		Ty* get( void )       { return static_cast< Ty* >( m_data->get_ptr() ); }
		Ty* get( void ) const { return static_cast< Ty* >( m_data->get_ptr() ); }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		std::enable_if_t< std::is_base_of_v< Ty, Ot > || std::is_base_of_v< Ot, Ty >,
		cShared_Ref< Ot > > cast( void )
		{
			Ot* op = static_cast< Ot* >( m_data->get_ptr() );

			cShared_ptr< Ot > other{};
			other.m_data = m_data;
			inc();

			return other;
		}
	};

	template< class Ty >
	class cWeak_Ptr : public cPtr_base
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cWeak_Ptr;

		public:
		cWeak_Ptr( void )
		{
			m_data = nullptr;
		} // cShared_ptr

		cWeak_Ptr( nullptr_t )
		{
			m_data = nullptr;
		} // cShared_ptr

		cWeak_Ptr( const cWeak_Ptr& _right )
		{
			m_data = _right.m_data;
			inc_weak();

		} // cShared_ptr

		cWeak_Ptr( cWeak_Ptr&& _right ) noexcept
		{
			m_data = _right.m_data;
			_right.m_data = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cWeak_Ptr( const cWeak_Ptr< Other >& _right )
		{
			m_data = _right.m_data;
			inc_weak();
		} // cShared_ptr

		cWeak_Ptr( const cPtr_base& _right )
		{
			m_data = _right.m_data;

			if( m_data )
			{
				inc_weak();
			}
		} // cShared_ptr

		~cWeak_Ptr( void )
		{
			dec_weak();
		} // ~cShared_ptr

		auto& operator=( const cWeak_Ptr& _right )
		{
			if( m_data != _right.m_data && this != &_right )
			{
				dec_weak();
				m_data = _right.m_data;
				inc_weak();
			}

			return *this;
		}

		auto& operator=( cWeak_Ptr&& _right ) noexcept
		{
			if( m_data != _right.m_data )
			{
				dec_weak();
				m_data = _right.m_data;
			}
			_right.m_data = nullptr;

			return *this;
		}

		auto& operator=( const cShared_ptr< Ty >& _right )
		{
			if( m_data != _right.m_data )
			{
				dec_weak();
				m_data = _right.m_data;
				inc_weak();
			}

			return *this;
		}

		auto& operator=( cShared_ptr< Ty >&& _right ) noexcept
		{
			dec_weak(); // TODO: Make sure that it's counting correctly.
			m_data = _right.m_data;
			_right.m_data = nullptr;
			dec(); // Decrease due to stealing shared ptr

			return *this;
		}


		auto& operator=( std::nullptr_t )
		{
			dec_weak();
			m_data = nullptr;

			return *this;
		}

		template< class Other >
		requires std::is_convertible_v< Other, Ty >
		auto& operator=( const cShared_ptr< Other >& _right )
		{
			dec_weak();
			m_data = _right.m_data;
			inc_weak();

			return *this;
		} // operator= (Copy Shared)

		template< class Other >
		requires ( std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other, Ty > )
		auto& operator=( const cShared_ptr< Other >& _right )
		{
			dec_weak();
			m_data = _right.m_data;
			inc_weak();

			return *this;
		} // operator= (Copy)

		operator bool( void ) const { return is_valid(); }

		Ty& operator  *( void ){ return *get(); }
		Ty* operator ->( void ){ return  get(); }

		Ty& operator  *( void ) const { return *get(); }
		Ty* operator ->( void ) const { return  get(); }

		operator Ty*   ( void )       { return get(); }
		operator Ty*   ( void ) const { return get(); }

		Ty* get( void )       { return static_cast< Ty* >( m_data->get_ptr() ); }
		Ty* get( void ) const { return static_cast< Ty* >( m_data->get_ptr() ); }

		bool is_valid( void ) const { return m_data != nullptr && m_data->get_ptr(); }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		requires std::is_convertible_v< Ty, Ot >
		cWeak_Ptr< Ot > cast( void )
		{
			cShared_ptr< Ot > other{};
			other.m_data = m_data;
			inc_weak();

			return other;
		} // cast

		// Only use if you KNOW that it's safe to use a non-shared ptr as a weak ptr.
		static auto make_unsafe( Ty* _ptr )
		{
			cWeak_Ptr unsafe;
			unsafe.m_data = QW_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			unsafe.inc_weak();

			return unsafe;
		} // make_unsafe
	};

	template< class Ty >
	class cShared_from_this
	{
	public:
		// Warning, don't run this in the constructor.
		cShared_ptr< Ty > get_shared_this( void )       { return m_self; }
		// Warning, don't run this in the constructor.
		cShared_ptr< Ty > get_shared_this( void ) const { return m_self; }
		cWeak_Ptr  < Ty > get_weak_this  ( void )       { return m_self; }
		cWeak_Ptr  < Ty > get_weak_this  ( void ) const { return m_self; }

	protected:
		cShared_from_this( void )
		: m_self( cPtr_base( QW_SINGLE( Ptr_logic::cData< Ty >, static_cast< Ty* >( this ) ) ) )
		{
		} // cShared_from_this

	private:
		cPtr_base m_self;
	};

	template< class Ty, class... Args >
	auto make_shared( Args&&... _args ) -> cShared_ptr< Ty >
	{
		// TODO: Add protection for cShared_from_this, if get_shared_this is ran in constructor, it deletes itself.
		Ty* ptr = QW_SINGLE( Ty, std::forward< Args >( _args )... );

		if constexpr( std::is_base_of_v< cShared_from_this< Ty >, Ty > )
			return ptr->get_shared_this();
		else
			return cShared_ptr< Ty >( ptr );
	}
} // qw::