/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <map>

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

			kUpdate   = 0x01,
			kRender   = 0x02,
			kUpdateRender = kUpdate | kRender,

			kEnabled  = 0x04,
			kDisabled = 0x10,
			kEnabledDisabled = kEnabled | kDisabled,

			kDebugRender = 0x20,
			kAllDebug    = kDebugRender, // TOOD: Add more debug events

			kAll    = 0xffff,
		};
	} // Object

	template<>
	class hash< Object::eEvents > : public Hashing::iHashed
	{
		union event_hash_pair
		{
			uint64_t hash;
			struct{ uint32_t type; uint32_t value; };
		};
	public:
		constexpr hash( const char* _to_hash )
		: iHashed( 0 )
		{
			event_hash_pair pair;
			pair.type  = 0;
			pair.value = Hashing::fnv1a_32( _to_hash );
			m_hash = pair.hash;
		}

		constexpr hash( const Object::eEvents _event )
		: iHashed( 0 )
		{
			event_hash_pair pair;
			pair.type  = 1;
			pair.value = static_cast< uint32_t >( _event );
			m_hash = pair.hash;
		}

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
		template< class... Types >
		constexpr size_t kTotalSize = total_size< Types... >::value;

		class iEventParser
		{
		public:
			virtual ~iEventParser( void ) = default;

			virtual void push_event( void*  _event_data ) = 0;
		};

		template< class... Args >
		class cEventParser : public iEventParser
		{
			std::function< void( Args... ) > m_function;
		public:
			explicit cEventParser( const std::function< void( Args... ) >& _function )
			: m_function( _function )
			{
			} // cEventDispatcher

			void push_event( void* _event_data ) override
			{
				auto& tuple = *static_cast< std::tuple< Args... >* >( _event_data );

				std::apply( m_function, tuple );
			} // push_event
		};

		template<>
		class cEventParser< void > : public iEventParser
		{
			std::function< void() > m_function;
		public:
			explicit cEventParser( const std::function< void( void ) >& _function )
			: m_function( _function )
			{
			} // cEventDispatcher

			void push_event( void* ) override
			{
				// Void, nothing special is needed.
				m_function();
			} // push_event
		};

		class cEventDispatcher
		{
			std::map< size_t, iEventParser* > m_parsers;
			size_t m_arg_size;
		public:
			 cEventDispatcher( const size_t _arg_size ) : m_arg_size( _arg_size ){}
			~cEventDispatcher( void );

			size_t get_arg_size( void ) const { return m_arg_size; }

			template< class... Args >
			size_t add_listener( const std::function< void( Args... ) >& _listener )
			{
				auto id = m_parsers.size();
				auto parser = new cEventParser< Args... >( _listener );
				m_parsers.insert( std::make_pair( id, parser ) );
				return id;
			} // add_listener
			void   remove_listener( const size_t _id );

			template< class... Args >
			void push_event( Args... _args )
			{
				std::tuple< Args... > tuple{ _args... };
				for( auto& callback : m_parsers )
				{
					callback.second->push_event( &tuple );
				}
			} // push_event
		};

	} // Event

	class cEventManager : public cSingleton< cEventManager >
	{

	public:
		// TODO: Logic?
		 cEventManager( void );
		~cEventManager( void );

	template< class Ty, class... Args >
	std::pair< bool, size_t > registerLister( const hash< Object::eEvents >& _identity, const std::function< void( Ty*, Args... ) >& _function )
	{
		if( const auto itr = m_dispatcher.find( _identity ); itr != m_dispatcher.end() )
		{
			if( itr->second->get_arg_size() != Event::kTotalSize< Args... > )
			{
				printf( "Invalid size of arguments." );
				return { false, 0 };
			}

			// Still unsafe if not same size, but no convenient way to check.

			auto dispatcher = itr->second;
			return dispatcher->add_listener( _identity, _function );
		}

		auto dispatcher = new Event::cEventDispatcher( Event::kTotalSize< Args... > );
		m_dispatcher.insert( std::make_pair( _identity, dispatcher ) );
		return { true, 0 };
	} // registerLister

	template< class Ty >
	std::pair< bool, size_t > registerLister( const hash< Object::eEvents >& _identity, const std::function< void( Ty* ) >& _function )
	{
		if( const auto itr = m_dispatcher.find( _identity ); itr != m_dispatcher.end() )
		{
			if( itr->second->get_arg_size() != 0 )
			{
				printf( "Invalid size of arguments." );
				return { false, 0 };
			}

			// Still unsafe if not same size, but no convenient way to check.

			auto dispatcher = itr->second;
			auto function = _function;
			return { true, dispatcher->add_listener( function ) };
		}

		auto dispatcher = new Event::cEventDispatcher( 0 );
		m_dispatcher.insert( std::make_pair( _identity, dispatcher ) );
		return { true, 0 };
	} // registerLister

	std::pair< bool, size_t > registerLister( const hash< Object::eEvents >& _identity, const std::function< void( void ) >& _function )
	{
		if( const auto itr = m_dispatcher.find( _identity ); itr != m_dispatcher.end() )
		{
			if( itr->second->get_arg_size() != 0 )
			{
				printf( "Invalid size of arguments." );
				return { false, 0 };
			}

			// Still unsafe if not same size, but no convenient way to check.

			auto dispatcher = itr->second;
			auto function = _function;
			return { true, dispatcher->add_listener( function ) };
		}

		auto dispatcher = new Event::cEventDispatcher( 0 );
		m_dispatcher.insert( std::make_pair( _identity, dispatcher ) );
		return { true, 0 };
	} // registerLister

	template< class... Args >
	bool postEvent( const hash< Object::eEvents >& _event, Args... _args )
	{
		std::tuple< Args... > tuple{ _args... };

		const auto itr = m_dispatcher.find( _event );

		if( itr == m_dispatcher.end() )
		{
			printf( "Event not registered." );
			return false;
		}

		if( itr->second->get_arg_size() != Event::kTotalSize< Args... > )
		{
			printf( "Invalid size of arguments." );
			return false;
		}

		itr->second->push_event( &tuple );

		return true;
	} // postEvent

	bool postEvent( const hash< Object::eEvents >& _event )
	{
		const auto itr = m_dispatcher.find( _event );

		if( itr == m_dispatcher.end() )
		{
			printf( "Event not registered." );
			return false;
		}

		if( itr->second->get_arg_size() != 0 )
		{
			printf( "Invalid size of arguments." );
			return false;
		}

		itr->second->push_event( nullptr );

		return true;
	} // postEvent

	void unregisterEvent( const hash< Object::eEvents >& _event, const size_t& _id );

	private:
		std::unordered_map< hash< Object::eEvents >, Event::cEventDispatcher* > m_dispatcher = {};

	};
} // qw::
