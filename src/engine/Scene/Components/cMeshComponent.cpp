 /*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cMeshComponent.h"

#include "cCameraComponent.h"
#include "Assets/cTexture.h"
#include "Graphics/cRenderer.h"
#include "Scene/Managers/cSceneManager.h"

namespace qw::Object::Components
{
	cMeshComponent::cMeshComponent( const cShared_ptr<Assets::cMesh>& _mesh )
	: m_mesh( _mesh )
	, m_texture( Graphics::cRenderer::getWhiteTexture() )
	{
	} // cMeshComponent

	void cMeshComponent::render( void )
	{
		auto& back = cSceneManager::get_active_context()->getBack();

		m_transform.update();

		// Removed due to NDA

		m_mesh->renderMesh( &back );
	}
} // qw::Object::Components

