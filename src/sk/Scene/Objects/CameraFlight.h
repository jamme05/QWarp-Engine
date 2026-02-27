/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Input/Input.h>
#include <sk/Scene/Objects/Camera.h>

namespace sk::Object
{
	SK_OBJECT_CLASS( CameraFlight, cCamera ), public Input::iListener
	{
		SK_CLASS_BODY( CameraFlight )
	sk_public:
		cCameraFlight();
		explicit cCameraFlight( const std::string& _name, const float _movement_speed = 5.0f, const float _rotation_speed = 40.0f );
		explicit cCameraFlight( cSerializedObject& _object );
		~cCameraFlight( void ) override = default;

		Input::response_t onInput( const uint32_t _type, const Input::sEvent& _event ) override;

		void update( void ) override;

		void handleStickEvent ( const Input::sEvent& _event );
		void handleButtonEvent( const Input::sEvent& _event, const bool _up );
		void handleKeyEvent   ( const Input::sEvent& _event, const bool _up );
		void handleMouseEvent ( const Input::sEvent& _event );

		auto Serialize() -> cSerializedObject override;

		cVector3f m_position = kZero;
		cVector2f m_rotation = kZero;
		cVector2f m_speeds;
	};
} // sk::Object

SK_DECLARE_CLASS( sk::Object::CameraFlight )
