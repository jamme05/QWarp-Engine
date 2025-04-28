/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cCameraFlight.h"

#include "Platform/time.h"

namespace qw::Object
{
	cCameraFlight::cCameraFlight( const std::string& _name, const float _movement_speed, const float _rotation_speed )
	: cCamera( _name )
	, iListener( Input::kStick | Input::kButton, 1, true )
	, m_speeds( _movement_speed, _rotation_speed )
	{
		RegisterListener< cCameraFlight >( kUpdate, &cCameraFlight::update );
	} // cCameraFlight

	bool cCameraFlight::onInput( const Input::eType _type, const Input::sEvent& _event )
	{
		switch( _type )
		{
		case Input::kStick:       handleStickEvent ( _event );        break;
		case Input::kButton_Down: handleButtonEvent( _event, false ); break;
		case Input::kButton_Up:   handleButtonEvent( _event, true  ); break;
		default: break;
		}

		return false;
	} // onInput

	void cCameraFlight::update( void )
	{
		// TODO: Make transform vector.
		getPosition() += Time::Delta * m_speeds.x * ( getTransform().getWorldRight() * m_position.x + getTransform().getWorldUp() * m_position.y + getTransform().getWorldFront() * m_position.z );
		getRotation() += Time::Delta * m_speeds.y * cVector3f{ m_rotation.x, m_rotation.y, 0.0f };
		getTransform().update();
		cCamera::update();
	} // update

	void cCameraFlight::handleStickEvent( const Input::sEvent& _event )
	{
		// Stick event will always have analog, so this is safe.
		const auto movement = _event.pad->current_stick - _event.pad->previous_stick;
		switch( _event.pad->analog )
		{
		case Input::eAnalog::kLeft:
		{
			m_position += { -movement.x, 0.0f, -movement.y };
		}
		break;
		case Input::eAnalog::kRight:
		{
			m_rotation += { -movement.y, movement.x };
		}
		break;
		default: break;
		}
	} // handleStickEvent

	void cCameraFlight::handleButtonEvent( const Input::sEvent& _event, const bool _up )
	{
		if( _up )
		{
			switch( _event.pad->button )
			{
			case Input::eButton::kR3:
			case Input::eButton::kL3: m_position.y = 0.0f; break;
			default: break;
			}
		}
		else
		{
			switch( _event.pad->button )
			{
			case Input::eButton::kR3: m_position.y =  1.0f; break;
			case Input::eButton::kL3: m_position.y = -1.0f; break;
			default: break;
			}
		}
	}
} // qw::Object::

