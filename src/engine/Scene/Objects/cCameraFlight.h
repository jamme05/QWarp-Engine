/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "cCamera.h"
#include "Input/cInput.h"

namespace qw::Object
{
	QW_OBJECT_CLASS( CameraFlight, cCamera ), public Input::iListener
	{
	public:
		 cCameraFlight( const std::string& _name, const float _movement_speed = 5.0f, const float _rotation_speed = 40.0f );
		~cCameraFlight( void ) = default;

		bool onInput( const Input::eType _type, const Input::sEvent& _event ) override;

		void update( void ) override;

		void handleStickEvent ( const Input::sEvent& _event );
		void handleButtonEvent( const Input::sEvent& _event, const bool _up );

		cVector3f m_position = kZero;
		cVector2f m_rotation = kZero;
		cVector2f m_speeds;
	};
} // qw::Object