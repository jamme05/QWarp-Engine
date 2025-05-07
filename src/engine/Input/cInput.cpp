/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Input/cInput.h"

#include <mutex>

#include "Memory/Tracker/cTracker.h"

namespace qw
{
	namespace Input
	{
		namespace
		{
			// TODO: Replace with map
			std::vector< iListener* > listeners = { };
			std::mutex                mtx;
#if defined( DEBUG )
			bool                      log_inputs = false;
#endif // DEBUG
		} // Table

		void setLogInputs( const bool _log_inputs )
		{
#if defined( DEBUG )
			log_inputs = _log_inputs;
#else // DEBUG
			( void ) _log_inputs;
#endif // !DEBUG
		} // setLogInputs

		void addListener( iListener* _listener )
		{
			std::lock_guard lock( mtx );

			auto itr = std::ranges::find_if( listeners , [ _listener ]( const iListener* _l ){ return _l->getPriority() > _listener->getPriority(); } );
			if( itr != listeners.end() )
				listeners.insert( itr, _listener );
			else
				listeners.push_back( _listener );

		} // addListener

		void removeListener( const iListener* _listener )
		{
			std::lock_guard lock( mtx );

			if( const auto itr = std::find( listeners.begin(), listeners.end(), _listener ); itr != listeners.end() )
				listeners.erase( itr );

		} // addListener

		void event( const eType _type, const sPadEvent& _event )
		{
			event( _type, { &_event } );

		} // event

		void event( const eType _type, const sEvent& _event )
		{
#if defined( DEBUG )
			if( log_inputs )
			{
				// If it's a pad event.
				if( _event.pad )
				{
					const char* target_analog = _event.pad->analog == eAnalog::kLeft ? "Left " : "Right";

					switch( _type )
					{
					case eType::kStick:       printf( "Stick       Event: %s Stick, %.2f %.2f \n", target_analog, _event.pad->current_stick.x, _event.pad->current_stick.y ); break;
					case eType::kButton_Down: printf( "Button_Down Event: %6d \n", static_cast< int32_t >( _event.pad->button ) ); break;
					case eType::kButton_Up:   printf( "Button_Up   Event: %6d \n", static_cast< int32_t >( _event.pad->button ) ); break;
					case eType::kAnalog:      printf( "Analog      Event: %s %.2f \n", target_analog, _event.pad->current_analog ); break;
					default:                  printf( "Developer too lazy to implament" ); break;
					}
				}
			}
#endif // DEBUG

			for( const auto& listener : listeners )
			{
				if( listener->getEnabled() && listener->getFilter() & _type )
				{
					if( listener->onInput( _type, _event ) )
						break;
				}
			}

		} // event

		iListener::iListener( const uint32_t _filter, const uint16_t _priority, const bool _enabled )
		: m_filter( _filter )
		, m_priority( _priority )
		, m_enabled( _enabled )
		{
			Input::addListener( this );

		} // iListener

		iListener::~iListener( void )
		{
			Input::removeListener( this );

		} // ~iListener

	} // Input::

} // qw::
