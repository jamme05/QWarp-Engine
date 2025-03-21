/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cMesh.h"

#include "cCamera.h"
#include "Assets/cTexture.h"
#include "Graphics/cRenderer.h"
#include "Scene/Managers/cSceneManager.h"

namespace qw::Object::Components
{
	void cMesh::render( void )
	{
		auto& back = cSceneManager::get_active_context()->getBack();

		m_transform.update();

		// TODO: Make a wrapper for this:
		// Rendering removed due to NDA

		m_mesh->renderMesh( &back );
	}
} // qw::Object::Components

