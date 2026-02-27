/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Assets/Material.h>
#include <sk/Assets/Mesh.h>
#include <sk/Scene/Components/Component.h>

namespace sk::Assets
{
	class cMaterial;
} // sk::Assets::

namespace sk::Scene
{
	class cCameraManager;
} // sk::Scene

namespace sk::Object::Components
{
	SK_COMPONENT_CLASS( MeshComponent )
	{
		SK_CLASS_BODY( MeshComponent )
	public:
		cMeshComponent();
		explicit cMeshComponent( const cShared_ptr< cAsset_Meta >& _mesh, const cShared_ptr< cAsset_Meta >& _material );
		explicit cMeshComponent( cSerializedObject& _object );

		void enabled() override;
		void disabled() override;
		bool IsReady() const;

		auto GetMesh() -> Assets::cMesh*;
		auto GetMaterial() -> Assets::cMaterial*;
		void SetMesh( const cShared_ptr< cAsset_Meta >& _mesh );
		void SetMaterial( const cShared_ptr< cAsset_Meta >& _material );

		auto Serialize() -> cSerializedObject override;

	private:
		cAsset_Ref< Assets::cMesh, eAsset_Ref_Mode::kManual >     m_mesh_;
		cAsset_Ref< Assets::cMaterial, eAsset_Ref_Mode::kManual > m_material_;
	};
} // sk::Object::Components

SK_DECLARE_CLASS( sk::Object::Components::MeshComponent )