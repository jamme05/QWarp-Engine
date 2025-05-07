/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cCamera.h"
#include "Scene/Components/cCameraComponent.h"

#include "Graphics/cRenderer.h"
#include "Scene/Managers/cCameraManager.h"

namespace qw::Object
{
	cCamera::cCamera( const std::string& _name )
	: cObject( _name )
	{
		const auto& renderer = Graphics::cRenderer::get();
		auto& resolution     = renderer.getWindowResolution();
		const auto  aspect   = renderer.getWindowAspectRatio();

		const Graphics::sViewport viewport{ 0, 0, resolution.x, resolution.y };
		const Graphics::sScissor  scissor = viewport;

		Components::cCameraComponent::sCameraSettings settings{ 70.0f, aspect, 0.1f, 2000.0f };

		m_camera = addComponent< Components::cCameraComponent >( viewport, scissor, settings, Components::cCameraComponent::eType::kPerspective );
		m_root   = m_camera;
		// Update root as old root has been deleted.
		m_camera->setParent( nullptr );
	}

	void cCamera::setAsMain( void ) const
	{
		Scene::cCameraManager::get().setMainCamera( m_camera );
	} // setAsMain
} // qw::Object::