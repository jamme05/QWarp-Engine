/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Scene/cObject.h"
#include "Scene/Components/cCameraComponent.h"

namespace qw::Object
{
	QW_OBJECT_CLASS( Camera )
	{
	public:
		explicit cCamera( const std::string& _name );

		void setAsMain( void ) const;

		Components::CameraComponent::ptr_t m_camera;
	};
} // qw::Object