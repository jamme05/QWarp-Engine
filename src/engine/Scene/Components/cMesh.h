/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "iComponent.h"
#include "Assets/Manager/cAsset_manager.h"
#include "Graphics/cRender_context.h"
#include "Math/cMatrix4x4.h"
#include "Types/graphics_types.h"
#include "Assets/cMesh.h"

namespace qw::Assets {
	class cTexture;
}

namespace qw::Scene
{
	class cCameraManager;
} // qw::Scene

namespace qw::Object::Components
{
	QW_COMPONENT_CLASS( Mesh )
	{
	public:
		explicit cMesh( Assets::Mesh::Shared_t _mesh )
		: m_mesh( std::move( _mesh ) )
		{}

		void render( void ) override;

		void setTexture( const cShared_ptr< Assets::cTexture >& _texture ){	m_texture = _texture; }

	private:
		cShared_ptr< Assets::cMesh > m_mesh = nullptr;
		// TODO: Move to materal
		cShared_ptr< Assets::cTexture > m_texture = nullptr;
	};
} // qw::Object::Components
