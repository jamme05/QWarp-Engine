/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Scene/cObject.h"
#include "Scene/Components/cCamera.h"

namespace qw::Object
{
	class cCamera : public iObject
	{
	public:
		explicit cCamera( std::string _name );

		void setAsMain( void ) const;

		Components::Camera::Shared_t m_camera;
	};
} // qw::Object