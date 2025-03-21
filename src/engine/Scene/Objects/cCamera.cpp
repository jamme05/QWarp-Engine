/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cCamera.h"
#include "Scene/Components/cCamera.h"

#include "Graphics/cRenderer.h"
#include "Scene/Managers/cCameraManager.h"

namespace qw::Object
{
	cCamera::cCamera( std::string _name )
	: iObject( std::move( _name ) )
	{
		const auto& renderer = Graphics::cRenderer::get();
		auto& resolution     = renderer.getWindowResolution();
		const auto  aspect   = renderer.getWindowAspectRatio();

		const Graphics::sViewport viewport{ 0, 0, resolution.x, resolution.y };
		const Graphics::sScissor  scissor = viewport;

		Components::cCamera::sCameraSettings settings{ 70.0f, aspect, 0.1f, 2000.0f };

		m_camera = addComponent< Components::cCamera >( viewport, scissor, settings, Components::cCamera::eType::kPerspective );
	}

	void cCamera::setAsMain( void ) const
	{
		Scene::cCameraManager::get().setMainCamera( m_camera );
	} // setAsMain
} // qw::Object::