/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Memory/Tracker/Tracker.h>
#include <sk/Misc/Offsetof.h>

#include <atomic>
#include <memory>

// TODO: Move shared pointers to Memory folder

namespace sk::Object
{
	class iComponent;
} // sk::Object::

namespace sk
{
	namespace Ptr_logic
	{
		class cData_base
		{
			virtual void  deleteSelf( void ) = 0;
			virtual void  deleteCont( void ) = 0;
		protected:
			cData_base()
			: m_ref_count( 0 )
			, m_weak_ref_count( 0 )
			{}

			virtual ~cData_base() = default;

		public:
			virtual void* get_ptr() = 0;

			void inc()
			{
				++m_ref_count;
			}

			void inc_weak()
			{
				++m_weak_ref_count;
			}

			void dec()
			{
				if( --m_ref_count <= 0 && !m_is_deleting && m_is_constructed ) // TODO: Not use the bool and only change the ref count after deletion?
				{
					m_is_deleting = true;
					deleteCont();
					if( m_weak_ref_count <= 0 )
						deleteSelf();
					else
						m_is_deleting = false;
				}
			}

			void dec_weak()
			{
				if( --m_weak_ref_count <= 0 && m_ref_count <= 0 && !m_is_deleting && m_is_constructed )
				{
					deleteSelf();
				}
			}

			void completed()
			{
				m_is_constructed = true;
			}
		private:
			std::atomic_uint32_t m_ref_count;
			std::atomic_uint32_t m_weak_ref_count;
			// Will make sure that it doesn't get deleted multiple times.
			std::atomic_bool     m_is_deleting    = false;
			// Will make sure that it doesn't get deleted before being constructed.
			std::atomic_bool     m_is_constructed = false;
		};

		template< class Ty >
		class cData : public cData_base
		{
			void deleteCont( void ) override
			{
				SK_DELETE( m_ptr );
				m_ptr = nullptr;
			}
			void deleteSelf( void ) override
			{
				SK_DELETE( this );
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

		template< class Ty >
		class cCompactData : public cData_base
		{
			// template< sk::sMemberVariable Member, class Class >
			// friend constexpr int offset_of();
			void deleteCont() override
			{
				m_instance_->~Ty();
			}
			void deleteSelf() override
			{
				SK_DELETE( this );
			}
		public:
			void* get_ptr() override { return m_instance_; }

			template< class... Args >
			explicit cCompactData( Args&&... _args )
			: m_self_( this )
			, m_instance_( reinterpret_cast< Ty* >( m_storage_ ) )
			{
				::new( m_storage_ ) Ty( std::forward< Args >( _args )... );
				completed();
			}

			~cCompactData() override = default;

		private:
			Ty*         m_instance_;
			cData_base* m_self_;
		public:
			std::byte   m_storage_[ sizeof( Ty ) ];
		};
		static constexpr auto kCompactDataOffset = offset_of< &cCompactData< size_t >::m_storage_ >();
	} // Ptr_logic::

	class cPtr_base
	{
	public:
		cPtr_base( void ) = default;

		cPtr_base( const cPtr_base& _other )
		{
			if( this != &_other )
				m_data_ = _other.m_data_;
		} // cPtr_base

		explicit cPtr_base( Ptr_logic::cData_base* _data )
		{
			m_data_ = _data;
		} // cPtr_base

		auto& operator=( const cPtr_base& _right )
		{
			if( this != &_right )
				m_data_ = _right.m_data_;

			return *this;
		}

		operator bool ( void ) const { return m_data_ != nullptr; }

		bool operator!=( const void*      _other ) const { return m_data_->get_ptr() != _other; }
		bool operator==( const void*      _other ) const { return m_data_->get_ptr() == _other; }
		bool operator!=( const cPtr_base& _other ) const { return m_data_ != _other.m_data_; }
		bool operator==( const cPtr_base& _other ) const { return m_data_ == _other.m_data_; }

	protected:
		void inc     ( void ) const { if( m_data_ ) m_data_->inc(); }
		void dec     ( void ) const { if( m_data_ ) m_data_->dec(); }
		void inc_weak( void ) const { if( m_data_ ) m_data_->inc_weak(); }
		void dec_weak( void ) const { if( m_data_ ) m_data_->dec_weak(); }

		Ptr_logic::cData_base* m_data_ = nullptr;

		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cShared_Ref;
		template< class Fy >
		friend class cWeak_Ptr;
		friend class iShared_From_this;
	};

	template< class Ty >
	class cShared_from_this;

	// Shared ptr using itself of the tracker. Use in case the class is humongous.
	template< class Ty >
	class cShared_ptr : cPtr_base
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cWeak_Ptr;

		Ty* m_ptr_ = nullptr;
	public:
		cShared_ptr( void ) = default;

		explicit cShared_ptr( Ty* _ptr )
		{
			m_data_ = SK_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			m_data_->completed();
			m_ptr_ = _ptr;
			inc();
		} // cShared_ptr

		cShared_ptr( nullptr_t )
		{
			m_data_ = nullptr;
			m_ptr_  = nullptr;
		} // cShared_ptr

		cShared_ptr( const cShared_ptr& _right )
		{
			m_data_ = _right.m_data_;
			m_ptr_  = _right.m_ptr_;
			inc();

		} // cShared_ptr

		cShared_ptr( cShared_ptr&& _right ) noexcept
		{
			m_data_ = _right.m_data_;
			m_ptr_  = _right.m_ptr_;
			_right.m_data_ = nullptr;
			_right.m_ptr_  = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cShared_ptr( const cShared_ptr< Other >& _right )
		{
			m_data_ = _right.m_data_;
			m_ptr_  = static_cast< Ty* >( _right.m_ptr_ );
			inc();
		} // cShared_ptr

		cShared_ptr( const cPtr_base& _right )
		{
			m_data_ = _right.m_data_;
			m_ptr_  = nullptr;

			if( m_data_ )
			{
				m_ptr_ = static_cast< Ty* >( m_data_->get_ptr() );
				inc();
			}
		} // cShared_ptr

		~cShared_ptr( void )
		{
			dec();
		} // ~cShared_ptr

		auto& operator=( const cShared_ptr& _right )
		{
			if( this != &_right && m_data_ != _right.m_data_ )
			{
				dec();
				m_data_ = _right.m_data_;
				m_ptr_  = _right.m_ptr_;
				inc();
			}

			return *this;
		}

		auto& operator=( cShared_ptr&& _right ) noexcept
		{
			if( m_data_ != _right.m_data_ )
			{
				dec();
				m_data_ = _right.m_data_;
				m_ptr_  = _right.m_ptr_;
			}
			_right.m_data_ = nullptr;
			_right.m_ptr_  = nullptr;

			return *this;
		}

		auto& operator=( std::nullptr_t )
		{
			dec();
			m_data_ = nullptr;
			m_ptr_  = nullptr;

			return *this;
		}

		template< class Other >
		requires std::is_convertible_v< Other*, Ty* >
		cShared_ptr& operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data_ = _right.m_data_;
			m_ptr_  = _right.m_ptr_;
			inc();

			return *this;
		}

		template< class Other >
		requires ( std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other*, Ty* > )
		cShared_ptr& operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data_ = _right.m_data;
			m_ptr_  = _right.m_ptr_;
			inc();

			return *this;
		}

		bool operator==( std::nullptr_t ) const
		{
			return m_data_ == nullptr;
		}

		bool operator==( const cShared_ptr& _right ) const
		{
			return m_data_ == _right.m_data_;
		}

		operator bool() const { return ( m_data_ != nullptr ); }

		Ty& operator  *(){ return *get(); }
		Ty* operator ->(){ return  get(); }

		Ty& operator  *() const { return *get(); }
		Ty* operator ->() const { return  get(); }

		explicit operator Ty*()       { return get(); }
		explicit operator Ty*() const { return get(); }

		Ty* get()       { return m_ptr_; }
		Ty* get() const { return m_ptr_; }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		requires ( std::is_base_of_v< Ty, Ot > || std::is_base_of_v< Ot, Ty > || std::is_same_v< Ty, void > )
		cShared_ptr< Ot > Cast( void )
		{
			if constexpr( std::is_same_v< Ty, Ot > )
			{
				return *this;
			}
			else
			{
				cShared_ptr< Ot > other{};
				other.m_data_ = m_data_;
				other.m_ptr_  = static_cast< Ot* >( m_ptr_ );
				inc();

				return other;
			}
		}
	};

	template< class Ty >
	class cShared_Ref : cPtr_base // TODO: add usages.
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cShared_Ref;
		template< class Fy >
		friend class cWeak_Ptr;

		public:
		cShared_Ref( void ) = default;

		explicit cShared_Ref( Ty* _ptr )
		{
			m_data_ = SK_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			inc();
		} // cShared_ptr

		cShared_Ref( const cShared_Ref& _right )
		{
			m_data_ = _right.m_data_;
			inc();

		} // cShared_ptr

		cShared_Ref( cShared_Ref&& _right ) noexcept
		{
			m_data_ = _right.m_data_;
			_right.m_data_ = nullptr;
		} // cShared_ptr

		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cShared_Ref( const cShared_ptr< Other >& _right )
		{
			m_data_ = _right.m_data;
			inc();
		} // cShared_ptr

		cShared_Ref( const cPtr_base& _right )
		{
			m_data_ = _right.m_data_;

			if( m_data_ )
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
			if( m_data_ != _right.m_data_ && this != &_right )
			{
				dec();
				m_data_ = _right.m_data_;
				inc();
			}

			return *this;
		}

		auto& operator=( cShared_Ref&& _right ) noexcept
		{
			if( m_data_ != _right.m_data_ )
			{
				dec();
				m_data_ = _right.m_data_;
			}
			_right.m_data_ = nullptr;

			return *this;
		}

		template< class Other >
		requires std::is_convertible_v< Other*, Ty* >
		cShared_Ref& operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data_ = _right.m_data;
			inc();

			return *this;
		}

		template< class Other >
		requires ( std::is_base_of_v< Ty, Other > && ! std::is_convertible_v< Other*, Ty* > )
		cShared_Ref& operator=( const cShared_ptr< Other >& _right )
		{
			dec();
			m_data_ = _right.m_data;
			inc();

			return *this;
		}

		operator bool( void ) const { return ( m_data_ != nullptr ); }

		Ty& operator  *( void ){ return *get(); }
		Ty* operator ->( void ){ return  get(); }

		Ty& operator  *( void ) const { return *get(); }
		Ty* operator ->( void ) const { return  get(); }

		operator Ty*   ( void )       { return get(); }
		operator Ty*   ( void ) const { return get(); }

		Ty* get( void )       { return static_cast< Ty* >( m_data_->get_ptr() ); }
		Ty* get( void ) const { return static_cast< Ty* >( m_data_->get_ptr() ); }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		requires ( std::is_base_of_v< Ty, Ot > || std::is_base_of_v< Ot, Ty > )
		cShared_Ref< Ot > cast( void )
		{
			// Why was this here?
			// Ot* op = static_cast< Ot* >( m_data->get_ptr() );

			cShared_ptr< Ot > other{};
			other.m_data = m_data_;
			inc();

			return other;
		}
	};

	template< class Ty >
	class cWeak_Ptr : cPtr_base
	{
		template< class Fy >
		friend class cShared_ptr;
		template< class Fy >
		friend class cWeak_Ptr;

		public:
		cWeak_Ptr()
		{
			m_data_ = nullptr;
		} // cShared_ptr

		cWeak_Ptr( nullptr_t )
		{
			m_data_ = nullptr;
		} // cShared_ptr

		cWeak_Ptr( const cWeak_Ptr& _right )
		{
			m_data_ = _right.m_data_;
			inc_weak();

		} // cShared_ptr

		cWeak_Ptr( cWeak_Ptr&& _right ) noexcept
		{
			m_data_ = _right.m_data_;
			_right.m_data_ = nullptr;
		} // cShared_ptr
		
		template< class Other, std::enable_if_t< std::is_base_of_v< Ty, Other > >... >
		cWeak_Ptr( const cWeak_Ptr< Other >& _right )
		{
			m_data_ = _right.m_data_;
			inc_weak();
		} // cShared_ptr

		cWeak_Ptr( const cShared_ptr< Ty >& _right )
		{
			m_data_ = _right.m_data_;

			if( m_data_ )
				inc_weak();
		}

		template< class Ot >
		requires std::is_base_of_v< Ty, Ot >
		cWeak_Ptr( const cShared_ptr< Ot >& _right )
		{
			m_data_ = _right.m_data_;

			if( m_data_ )
				inc_weak();
		}

		cWeak_Ptr( const cPtr_base& _right )
		{
			m_data_ = _right.m_data_;

			if( m_data_ )
			{
				inc_weak();
			}
		} // cShared_ptr

		~cWeak_Ptr( void )
		{
			dec_weak();
			m_data_ = nullptr;
		} // ~cShared_ptr

		auto& operator=( const cWeak_Ptr& _right )
		{
			if( m_data_ != _right.m_data_ && this != &_right )
			{
				dec_weak();
				m_data_ = _right.m_data_;
				inc_weak();
			}

			return *this;
		}

		auto& operator=( cWeak_Ptr&& _right ) noexcept
		{
			if( m_data_ != _right.m_data_ )
			{
				dec_weak();
				m_data_ = _right.m_data_;
			}
			_right.m_data_ = nullptr;

			return *this;
		}

		auto& operator=( const cShared_ptr< Ty >& _right )
		{
			if( m_data_ != _right.m_data_ )
			{
				dec_weak();
				m_data_ = _right.m_data_;
				inc_weak();
			}

			return *this;
		}

		auto& operator=( cShared_ptr< Ty >&& _right ) noexcept
		{
			dec_weak();
			m_data_ = std::move( _right ).m_data_;
			_right.m_data_ = nullptr;
			dec(); // Decrease due to stealing shared ptr

			inc_weak();

			return *this;
		}


		auto& operator=( std::nullptr_t )
		{
			dec_weak();
			m_data_ = nullptr;

			return *this;
		}

		template< class Other >
		requires std::is_convertible_v< Other*, Ty* >
		auto& operator=( const cShared_ptr< Other >& _right )
		{
			dec_weak();
			m_data_ = _right.m_data;
			inc_weak();

			return *this;
		} // operator= (Copy Shared)

		template< class Other >
		requires ( std::is_base_of_v< Ty, Other > && std::is_convertible_v< Other*, Ty* > )
		auto& operator=( const cShared_ptr< Other >& _right )
		{
			dec_weak();
			m_data_ = _right.m_data;
			inc_weak();

			return *this;
		} // operator= (Copy)

		[[ nodiscard ]] auto Lock() const
		{
			return cShared_ptr< Ty >( static_cast< const cPtr_base& >( *this ) );
		}

		explicit operator bool( void ) const { return is_valid(); }
		
		bool operator==( std::nullptr_t ) const { return !is_valid(); }
		bool operator==( const cWeak_Ptr& _right ) const { return m_data_ == _right.m_data_; }
		// TODO: Maybe compare the data used instead of the pointers?
		// May be safe tho.
		bool operator==( const cShared_ptr< Ty >& _right  ) const { return get() == _right.get(); }
		
		Ty& operator  *( void ){ return *get(); }
		Ty* operator ->( void ){ return  get(); }

		Ty& operator  *( void ) const { return *get(); }
		Ty* operator ->( void ) const { return  get(); }

		operator Ty*   ( void )       { return get(); }
		operator Ty*   ( void ) const { return get(); }

		Ty* get( void )       { return static_cast< Ty* >( m_data_ ? m_data_->get_ptr() : nullptr ); }
		Ty* get( void ) const { return static_cast< Ty* >( m_data_ ? m_data_->get_ptr() : nullptr ); }

		bool is_valid( void ) const { return get() != nullptr; }

		// TODO: Make safer version.
		// Safe enough? You can go up/down in the polymorphic family tree?

		// NOTE: USE WITH CARE, be sure that you know what you're doing.
		template< class Ot >
		requires std::is_base_of_v< Ty, Ot >
		cWeak_Ptr< Ot > Cast()
		{
			// TODO: Introduce a dynamic cast here
			cWeak_Ptr< Ot > other{};
			other.m_data_ = m_data_;
			inc_weak();

			return other;
		} // cast

		// Only use if you KNOW that it's safe to use a non-shared ptr as a weak ptr.
		static auto make_unsafe( Ty* _ptr )
		{
			cWeak_Ptr unsafe;
			unsafe.m_data_ = SK_SINGLE( Ptr_logic::cData< Ty >, _ptr );
			unsafe.inc_weak();

			return unsafe;
		} // make_unsafe
	};
	
	// We need this class as some classes will have different base classes.
	class iShared_From_this
	{
	protected:
		explicit iShared_From_this( Ptr_logic::cData_base* _data )
		: m_self_( _data )
		{}
		
		cPtr_base m_self_;
	};
	template< class Ty >
	class cShared_from_this : public iShared_From_this
	{
	public:
		// Warning, don't run this in the constructor.
		[[ nodiscard ]] auto get_shared( void )       -> cShared_ptr< Ty > { return m_self_; }
		// Warning, don't run this in the constructor.
		[[ nodiscard ]] auto get_shared( void ) const -> cShared_ptr< Ty > { return m_self_; }
		[[ nodiscard ]] auto get_weak  ( void )       -> cWeak_Ptr  < Ty > { return m_self_; }
		[[ nodiscard ]] auto get_weak  ( void ) const -> cWeak_Ptr  < Ty > { return m_self_; }

	protected:
		cShared_from_this()
		: iShared_From_this( *( reinterpret_cast< Ptr_logic::cData_base** >( static_cast< Ty* >( this ) ) - 1 ) )
		{} // cShared_from_this
	};

	// TODO: Replace cShared_ptr with std::shared_ptr to allow for optimizations.
	template< class Ty, class... Args >
	requires std::constructible_from< Ty, Args... >
	auto MakeShared_old( Args&&... _args ) -> cShared_ptr< Ty >
	{
		auto data = SK_SINGLE( Ptr_logic::cCompactData< Ty >, std::forward< Args >( _args )... );
		return cShared_ptr< Ty >( cPtr_base{ data } );
	}

	struct make_shared_helper
	{
		constexpr explicit make_shared_helper( const std::source_location& _location = std::source_location::current() )
		: location( _location )
		{}

		template< class Ty, class... Args >
		requires std::constructible_from< Ty, Args... >
		auto impl( Args&&... _args ) -> cShared_ptr< Ty >
		{
			auto data = Memory::alloc< Ptr_logic::cCompactData< Ty > >( 1, location, std::forward< Args >( _args )... );
			return cShared_ptr< Ty >( cPtr_base{ data } );
		}
		const std::source_location location;
	};

	// TODO: Move to it's own file
	template< class Ty >
	consteval auto& ceval( const Ty& _val ){ return _val; }
	template< class Ty >
	consteval auto ceval( Ty&& _val ){ return std::move( _val ); }
} // sk::

#define MakeShared ceval( sk::make_shared_helper{} ).impl
