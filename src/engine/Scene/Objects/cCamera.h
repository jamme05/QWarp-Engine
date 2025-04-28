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
	class cCamera : public iObject
	{
	public:
		explicit cCamera( const std::string& _name );

		void setAsMain( void ) const;

		Components::CameraComponent::Shared_t m_camera;
	};
} // qw::Object