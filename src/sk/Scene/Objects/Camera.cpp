/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Camera.h"

#include <framework/App.h>
#include <sk/Scene/Components/CameraComponent.h>
#include <sk/Scene/Managers/CameraManager.h> 
#include <sk/Window/Window.h>

namespace sk::Object
{
	cCamera::cCamera() : cCamera( "Camera" ){}

	cCamera::cCamera( const std::string& _name )
	: cObject( _name )
	{
		const auto  main_window = Platform::GetMainWindow();
		const auto  resolution   = main_window->GetResolution();
		const auto  aspect       = main_window->GetAspectRatio();

		const Graphics::sViewport viewport = { .x = 0, .y = 0, .width = resolution.x, .height = resolution.y };
		const Graphics::sScissor  scissor  = { .x = 0, .y = 0, .width = resolution.x, .height = resolution.y };

		Components::cCameraComponent::sCameraSettings settings{
			.auto_resize = true,
			.fov = 70.0f,
			.aspect = 0.0f,
			.near = 0.1f,
			.far = 2000.0f
		};

		m_camera_ = CreateComponent< Components::cCameraComponent >( viewport, scissor, settings, Components::cCameraComponent::eType::kPerspective );
		SetRoot( m_camera_ );
	}

	cCamera::cCamera( cSerializedObject& _object )
	: cObject( _object.GetBase< cObject >().value() )
	{}

	void cCamera::setAsMain( void ) const
	{
		Scene::cCameraManager::get().setMainCamera( m_camera_ );
	} // setAsMain

	auto cCamera::Serialize() -> cSerializedObject
	{
		cSerializedObject object( this );
		object.AddBase( cObject::Serialize() );
		object.EndWrite();
		return object;
	}
} // sk::Object::
