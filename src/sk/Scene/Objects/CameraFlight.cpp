/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "CameraFlight.h"

#include <sk/Input/Gamepad.h>
#include <sk/Input/Keyboard.h>
#include <sk/Platform/Time.h>

namespace sk::Object
{
	cCameraFlight::cCameraFlight( const std::string& _name, const float _movement_speed, const float _rotation_speed )
	: cCamera( _name )
	, iListener( Input::kStick | Input::kButton | Input::kKey | Input::kMouseButton | Input::kMouseRelative, 1, true )
	, m_speeds( _movement_speed, _rotation_speed )
	{
		RegisterListener< cCameraFlight >( kUpdate, &cCameraFlight::update );
	} // cCameraFlight

	cCameraFlight::cCameraFlight( cSerializedObject& _object )
	: cCamera( _object.GetBase< cCamera >().value() )
	, iListener( Input::kStick | Input::kButton | Input::kKey | Input::kMouseButton | Input::kMouseRelative, 1, true )
	{
		_object.BeginRead( this );
		m_speeds.x = _object.ReadValue< float >( "movement_speed" ).value_or( 1.0f );
		m_speeds.y = _object.ReadValue< float >( "rotation_speed" ).value_or( 1.0f );
		_object.EndRead();
	}

	Input::response_t cCameraFlight::onInput( const uint32_t _type, const Input::sEvent& _event )
	{
		switch( _type )
		{
		case Input::kStick:         handleStickEvent ( _event );        break;
		case Input::kButton_Down:   handleButtonEvent( _event, false ); break;
		case Input::kButton_Up:     handleButtonEvent( _event, true  ); break;
		case Input::kKey_Down:      handleKeyEvent   ( _event, false ); break;
		case Input::kKey_Up:        handleKeyEvent   ( _event, true  ); break;
		case Input::kMouseRelative:
		case Input::kMouseButton:   handleMouseEvent ( _event );        break;
		default: break;
		}

		return true;
	} // onInput

	void cCameraFlight::update( void )
	{
		// TODO: Make transform vector.
		GetPosition() += Time::Delta * m_speeds.x * ( GetTransform().GetRight() * m_position.x + GetTransform().GetUp() * m_position.y + GetTransform().GetForward() * m_position.z );
		GetRotation() += Time::Delta * m_speeds.y * cVector3f{ m_rotation.x, m_rotation.y, 0.0f };
		GetTransform().MarkDirty();
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
			case Input::Gamepad::kRStick:
			case Input::Gamepad::kLStick: m_position.y = 0.0f; break;
			default: break;
			}
		}
		else
		{
			switch( _event.pad->button )
			{
			case Input::Gamepad::kRStick: m_position.y =  1.0f; break;
			case Input::Gamepad::kLStick: m_position.y = -1.0f; break;
			default: break;
			}
		}
	}

	void cCameraFlight::handleKeyEvent( const Input::sEvent& _event, const bool _up )
	{
		const float val = _up ? 0.0f : 1.0f;
		switch( _event.keyboard->key )
		{
		// Forward
		case Input::Keyboard::kW: m_position.z =  val; break;
		// Back
		case Input::Keyboard::kS: m_position.z = -val; break;
		// Left
		case Input::Keyboard::kA: m_position.x = -val; break;
		// Right
		case Input::Keyboard::kD: m_position.x =  val; break;
		// Up
		case Input::Keyboard::kE: m_position.y =  val; break;
		// Down
		case Input::Keyboard::kQ: m_position.y = -val; break;
		default: break;
		}
	}

	void cCameraFlight::handleMouseEvent( const Input::sEvent& _event )
	{
		m_rotation = _event.mouse->relative;
	}

	auto cCameraFlight::Serialize() -> cSerializedObject
	{
		cSerializedObject object( this );
		object.AddBase( cCamera::Serialize() );
		object.WriteValue( "movement_speed", m_speeds.x );
		object.WriteValue( "rotation_speed", m_speeds.y );
		object.EndWrite();
		return object;
	}
} // sk::Object::
