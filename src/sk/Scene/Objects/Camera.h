/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Scene/Object.h>
#include <sk/Scene/Components/CameraComponent.h>

namespace sk::Object
{
	SK_OBJECT_CLASS( Camera )
	{
		SK_CLASS_BODY( Camera )
	public:
		cCamera();
		explicit cCamera( const std::string& _name );
		explicit cCamera( cSerializedObject& _object );

		void setAsMain() const;

		auto Serialize() -> cSerializedObject override;

	protected:
		cShared_ptr< Components::cCameraComponent > m_camera_;
	};
} // sk::Object

SK_DECLARE_CLASS( sk::Object::Camera )