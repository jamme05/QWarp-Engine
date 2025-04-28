/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "iComponent.h"
#include "Assets/Manager/cAsset_manager.h"
#include "Graphics/cRender_context.h"
#include "Assets/cMesh.h"

namespace qw::Object::Components
{
	QW_COMPONENT_CLASS( TransformComponent )
	{
	public:
		 cTransformComponent( void ) = default;
		~cTransformComponent( void ) = default;
	};
} // qw::Object::Components
