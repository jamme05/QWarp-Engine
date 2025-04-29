/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <functional>

#include "Containers/map.h"
#include "Containers/vector.h"

#include <Misc/Hashing.h>
#include "Misc/cSingleton.h"
#include "Misc/Smart_ptrs.h"

namespace qw
{
	class cScene;

	namespace Object
	{
		enum eEvents : uint16_t // TODO: Create an event listener class which works for both objects and components
		{
			kNone     = 0x00,

			// Global Events
			kUpdate   = 0x01,
			kRender   = 0x02,
			kUpdateRender = kUpdate | kRender,

			// Entity based Events
			kEnabled  = 0x04,
			kDisabled = 0x10,
			kEnabledDisabled = kEnabled | kDisabled,

			// Debug Events
			kDebugRender = 0x20,
			kAllDebug    = kDebugRender,

			kAll    = 0xffff,
		};
	} // Object

	template<>
	class hash< Object::eEvents > : public Hashing::iHashed
	{
		static constexpr size_t int_offset = sizeof( uint32_t ) * 8;
	public:
		//constexpr hash( const char* _to_hash )
		//: iHashed( Hashing::fnv1a_32( _to_hash ) )
		//{
		//} // hash

		constexpr hash( const str_hash& _other )
		: iHashed( _other.getHash() )
		{
		} // hash

		constexpr hash( const Object::eEvents _event )
		: iHashed( ( 1ull << 32 ) | static_cast< uint32_t >( _event ) )
		{
		} // hash

		HASH_REQUIREMENTS( hash )
	};

	namespace Event
	{
		template< class... Types >
		struct total_size
		{
			static constexpr size_t value = sizeof( std::tuple< Types... > );
		};

		template<>
		struct total_size< void >
		{
			static constexpr size_t value = 0;
		};

		template<>
		struct total_size<>
		{
			static constexpr size_t value = 0;
		};
		template< class... Types >
		constexpr size_t kTotal_Size = total_size< Types... >::value;

		constexpr size_t kInvalid_Id = ~0llu;

		class iEventDispatcher
		{
		public:
			         iEventDispatcher( void ) : m_counter( 0 ){}
			virtual ~iEventDispatcher( void ) = default;

			virtual void remove_listener( const size_t _id ) = 0;

			virtual size_t get_arg_size( void ) const = 0;

		protected:
			// TODO: Have some better ID management.
			size_t           m_counter;
			std::atomic_bool m_is_pushing{ false };
		};

		template< class... Args >
		class cEventDispatcher : public iEventDispatcher
		{
			map< size_t, std::function< void( Args... ) > > m_listeners;      // Static Listeners
			map< size_t, std::function< bool( Args... ) > > m_weak_listeners; // Class Listeners, returns false if no longer valid.
			vector< size_t > m_to_remove;
		public:
			 cEventDispatcher( void ) = default;
			~cEventDispatcher( void ) override
			{
				m_listeners     .clear();
				m_weak_listeners.clear();
			} // ~cEventDispatcher

			size_t get_arg_size( void ) const override { return kTotal_Size< Args... >; }

			size_t add_listener( const std::function< void( Args... ) >& _listener )
			{
				auto id = m_counter;
				m_listeners.insert( std::make_pair( id, _listener ) );
				m_counter++;
				return id;
			} // add_listener

			size_t add_listener( const std::function< bool( Args... ) >& _listener )
			{
				auto id = m_counter;
				m_weak_listeners.insert( std::make_pair( id, _listener ) );
				m_counter++;
				return id;
			} // add_listener

			void remove_listener( const size_t _id ) override
			{
				if( m_is_pushing )
				{
					m_to_remove.push_back( _id );
					return;
				}

				if( const auto itr = m_listeners.find( _id ); itr != m_listeners.end() )
			 	{
			 		m_listeners.erase( _id );
				}
				else if( const auto itr_2 = m_weak_listeners.find( _id ); itr_2 != m_weak_listeners.end() )
				{
					m_weak_listeners.erase( _id );
				}
			} // remove_listener

			// Cleans out all events that were supposed to get deleted during the last event push.
			void clean_listeners( void )
			{
				for( auto& id : m_to_remove )
				{
					if( const auto itr = m_listeners.find( id ); itr != m_listeners.end() )
					{
						m_listeners.erase( id );
					}
					else if( const auto itr_2 = m_weak_listeners.find( id ); itr_2 != m_weak_listeners.end() )
					{
						m_weak_listeners.erase( id );
					}
				}
				m_to_remove.clear();
			} // clean_listeners

			void push_event( Args... _args )
			{
				clean_listeners();

				m_is_pushing = true;
				if constexpr( kTotal_Size< Args... > == 0 )
				{
					for( auto& callback : m_listeners )
					{
						callback.second();
					}
					for( auto& weak_listener : m_weak_listeners )
					{
						if( !weak_listener.second() )
							m_to_remove.push_back( weak_listener.first );
					}
				}
				else
				{
					std::tuple< Args... > tuple{ std::forward< Args >( _args )... };
					for( auto& callback : m_listeners )
					{
						std::apply( callback.second, tuple );
					}
					for( auto& weak_listener : m_weak_listeners )
					{
						if( !std::apply( weak_listener.second, tuple ) )
							m_to_remove.push_back( weak_listener.first );
					}
				}
				m_is_pushing = false;
			} // push_event
		};
	} // Event

	class cEventManager : public cSingleton< cEventManager >
	{

	public:
		// TODO: Logic?
		 cEventManager( void );
		~cEventManager( void );

	template< class... Args >
	std::pair< bool, size_t > registerLister( const hash< Object::eEvents >& _identity, void( *_function )( Args... ) )
	{
		std::function< void( Args... ) > function = _function;
		if( const auto itr = m_dispatcher.find( _identity ); itr != m_dispatcher.end() )
		{
			if( itr->second->get_arg_size() != Event::kTotal_Size< Args... > )
			{
				printf( "Invalid size of arguments." );
				return { false, 0 };
			}

			// Still unsafe if not same size, but no convenient way to check.

			auto dispatcher = static_cast< Event::cEventDispatcher< Args... >* >( itr->second );
			return std::make_pair( true, dispatcher->add_listener( function ) );
		}

		auto dispatcher = QW_SINGLE_EMPTY( Event::cEventDispatcher< Args... > );
		dispatcher->add_listener( function );
		m_dispatcher.emplace( _identity, dispatcher );
		return { true, 0 };
	} // registerLister


	template< class Ty, class... Args >
	std::pair< bool, size_t > registerLister( const hash< Object::eEvents >& _identity, void( Ty::*_function )( Args... ), const cWeak_Ptr< Ty >& _class )
	{
		std::function< bool( Args... ) > function = [ _class, _function ]( Args... _args ){ if( !_class.is_valid() ) return false; ( _class->*_function )( _args... ); return true; };
		if( const auto itr = m_dispatcher.find( _identity ); itr != m_dispatcher.end() )
		{
			if( itr->second->get_arg_size() != Event::kTotal_Size< Args... > )
			{
				printf( "Invalid size of arguments." );
				return { false, 0 };
			}

			// Still unsafe if not same size, but no convenient way to check.

			auto dispatcher = static_cast< Event::cEventDispatcher< Args... >* >( itr->second );
			return std::make_pair( true, dispatcher->add_listener( function ) );
		}

		auto dispatcher = QW_SINGLE_EMPTY( Event::cEventDispatcher<> );
		dispatcher->add_listener( function );
		m_dispatcher.emplace( _identity, dispatcher );
		return { true, 0 };
	} // registerLister

	template< class... Args >
	bool postEvent( const hash< Object::eEvents >& _event, Args... _args )
	{
		const auto itr = m_dispatcher.find( _event );

		if( itr == m_dispatcher.end() )
		{
			//printf( "Event not registered." );
			return false;
		}

		if( itr->second->get_arg_size() != Event::kTotal_Size< Args... > )
		{
			printf( "Invalid size of arguments. \n" );
			return false;
		}

		static_cast< Event::cEventDispatcher< Args... >* >( itr->second )->post_event( std::forward< Args >( _args )... );

		return true;
	} // postEvent

	bool postEvent( const hash< Object::eEvents >& _event )
	{
		const auto itr = m_dispatcher.find( _event );

		if( itr == m_dispatcher.end() )
		{
			//printf( "Event not registered. \n" );
			return false;
		}

		if( itr->second->get_arg_size() != 0 )
		{
			printf( "Invalid size of arguments., %lu \n", itr->second->get_arg_size() );
			return false;
		}

		static_cast< Event::cEventDispatcher<>* >( itr->second )->push_event();

		return true;
	} // postEvent

	void unregisterEvent( const hash< Object::eEvents >& _event, const size_t& _id );

	private:
		std::unordered_map< hash< Object::eEvents >, Event::iEventDispatcher* > m_dispatcher = {};

	};

	namespace Event
	{
		class cEventListener
		{
		public:
			         cEventListener( void ) = default;
			virtual ~cEventListener( void );

		protected:
			template< class Ty, class... Args >
			size_t RegisterListener( const hash< Object::eEvents >& _identity, void( Ty::*_function )( Args... ) )
			{
				if constexpr( !std::is_base_of_v< cEventListener, Ty > )
				{
					printf( "Using invalid class" );
					return kInvalid_Id;
				}

				cWeak_Ptr< Ty > weak_ptr;

				if constexpr( std::is_base_of_v< cShared_from_this< Ty >, Ty > )
					weak_ptr = static_cast< Ty* >( this )->get_weak_this();
				else // Thanks to it unregistering all events, this is a safe option.
					weak_ptr = cWeak_Ptr< Ty >::make_unsafe( static_cast< Ty* >( this ) );

				std::pair< bool, size_t > res = cEventManager::get().registerLister< Ty, Args... >( _identity, _function, weak_ptr );
				if( !res.first )
					return kInvalid_Id;

				m_listeners.insert( { _identity, { std::addressof( _function ), res.second } } );
				return res.second;
			} // RegisterEvent

			template< class Ty, class... Args >
			void UnregisterListener( const hash< Object::eEvents >& _identity, void( Ty::*_function )( Args... ) )
			{
				if( const auto id = FindListenerId( _identity, static_cast< void* >( _function ) ); id != kInvalid_Id )
					cEventManager::get().unregisterEvent( _identity, id );
				else
					printf( "Unregistering invalid listener." );
			} // UnregisterEvent

			template< class Ty, class... Args >
			std::pair< bool, size_t > FindListenerId( const hash< Object::eEvents >& _event, void( Ty::*_function )( Args... ) )
			{
				auto id = FindListenerId( _event, static_cast< void* >( _function ) );
				return { id != kInvalid_Id, id };
			} // FindListenerId

			static void UnregisterListener( const hash< Object::eEvents >& _event, size_t _id );

			void UnregisterListenersOfType( const hash< Object::eEvents >& _event );

		private:
			size_t FindListenerId( const hash< Object::eEvents >& _event, void* _raw_ptr );

			multimap< hash< Object::eEvents >, std::pair< void*, size_t > > m_listeners;
		};

	} // Event::

	template< class Ty, class... Args >
	size_t Register_Event_Helper( const hash< Object::eEvents >& _event, void( Ty::*_function )( Args... ), Ty* _class )
	{
		if constexpr( std::is_base_of_v< Event::cEventListener, Ty > )
		{
			return _class->RegisterListener( _event, _function );
		}
		else
		{
			static_assert( std::is_base_of_v< cShared_from_this< Ty >, Ty >, "This class must derive from cShared_from_this, or use a static event function." );
			return cEventManager::get().registerLister( _event, _function, _class->get_weak_this() ).second;
		}
	} // Register_Event_Helper

	template< class... Args >
	size_t Register_Event_Helper( const hash< Object::eEvents >& _event, void( *_function )( Args... ), void* )
	{
		return qw::cEventManager::get().registerLister( _event, _function ).second;
	} // Register_Event_Helper

} // qw::

#define EVENT_HASH( Event ) constexpr qw::hash< qw::Object::eEvents > Hash( Event )

#define POST_EVENT( Event ) { \
EVENT_HASH( Event ); \
qw::cEventManager::get().postEvent( Hash ); }

// Has to be called inside a class
#define REGISTER_LISTENER( Event, Function ) { \
EVENT_HASH( Event ); \
qw::Register_Event_Helper( Hash, Function, this ); }
