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
		cEventDispatcher::~cEventDispatcher( void )
		{
			for( auto& parser : m_parsers )
			{
				delete parser.second;
			}
			m_parsers.clear();
		} // ~cEventDispatcher

		void cEventDispatcher::remove_listener( const size_t _id )
		{
			if( const auto itr = m_parsers.find( _id ); itr != m_parsers.end() )
			{
				delete itr->second;
				m_parsers.erase( _id );
			}
		} // remove_listener
	} // Event

	cEventManager::cEventManager( void )
	{
	} // cEventManager

	cEventManager::~cEventManager( void )
	{
		for( const auto& dispatcher : m_dispatcher )
		{
			delete dispatcher.second;
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
} // qw::