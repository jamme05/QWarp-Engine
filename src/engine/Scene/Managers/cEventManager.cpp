/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */


#include "cEventManager.h"

namespace qw
{
	namespace Event
	{
	} // Event

	cEventManager::cEventManager( void )
	{
	} // cEventManager

	cEventManager::~cEventManager( void )
	{
		for( const auto& dispatcher : m_dispatcher )
		{
			QW_FREE( dispatcher.second );
		}
		m_dispatcher.clear();
	} // ~cEventManager

	void cEventManager::unregisterEvent( const hash< Object::eEvents >& _event, const size_t& _id )
	{
		const auto itr = m_dispatcher.find( _event );
		if( itr == m_dispatcher.end() )
			return;

		itr->second->remove_listener( _id );
	} // unregisterEvent

	Event::cEventListener::~cEventListener( void )
	{
		const auto event_manager = cEventManager::getPtr();
		if( event_manager == nullptr )
			return; // Return if event manager already has gotten destroyed.

		for( auto& listener : m_listeners )
		{
			event_manager->unregisterEvent( listener.first, listener.second.second );
		}
	} // ~cEventListener

	void Event::cEventListener::UnregisterListener( const hash< Object::eEvents >& _event, const size_t _id )
	{
		cEventManager::get().unregisterEvent( _event, _id );
	} // UnregisterEvent

	void Event::cEventListener::UnregisterListenersOfType( const hash<Object::eEvents>& _event )
	{
		auto& event_manager = cEventManager::get();
		auto range = m_listeners.equal_range( _event );
		for( auto it = range.first; it != range.second; ++it )
		{
			event_manager.unregisterEvent( _event, it->second.second );
		}
	} // UnregisterEventsOfType

	size_t Event::cEventListener::FindListenerId( const hash<Object::eEvents>& _event, void* _raw_ptr )
	{
		auto range = m_listeners.equal_range( _event );
		for( auto it = range.first; it != range.second; ++it )
		{
			if( it->second.first == _raw_ptr )
				return it->second.second;
		}

		return kInvalid_Id;
	} // FindListenerId
} // qw::