 /*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "MeshComponent.h"

#include <sk/Assets/Management/Asset_Manager.h>

 using namespace sk::Object::Components;

cMeshComponent::cMeshComponent( const cShared_ptr< cAsset_Meta >& _mesh, const cShared_ptr< cAsset_Meta >& _material )
: m_mesh_( get_weak() )
, m_material_( get_weak() )
{
	m_mesh_.SetAsset( _mesh );
	m_material_.SetAsset( _material );
}

cMeshComponent::cMeshComponent( cSerializedObject& _object )
: cComponent( _object.GetBase< iComponent >().value() )
{
	_object.BeginRead( this );
	m_mesh_     = _object.ReadData< cWeak_Ptr< cAsset_Meta > >( "mesh" )->Lock();
	m_material_ = _object.ReadData< cWeak_Ptr< cAsset_Meta > >( "material" )->Lock();
	m_mesh_ = cAsset_Manager::get().getAssetByName( "Cube.002" );
	m_material_ = cAsset_Manager::get().getAssetByName( "Material Test" );
	_object.EndRead();
}

void cMeshComponent::enabled()
{
	if( m_mesh_.IsValid() && !m_mesh_.IsLoaded() )
		m_mesh_.LoadAsync();
	if( m_material_.IsValid() && !m_material_.IsLoaded() )
		m_material_.LoadAsync();
}

void cMeshComponent::disabled()
{
	if( m_mesh_.IsValid() )
		m_mesh_.Unload();
	if( m_material_.IsValid() )
		m_material_.Unload();
}

bool cMeshComponent::IsReady() const
{
	return m_mesh_.IsLoaded() && m_material_.IsLoaded();
}

auto cMeshComponent::GetMesh() -> Assets::cMesh*
{
	return m_mesh_.Get();
}

auto cMeshComponent::GetMaterial() -> Assets::cMaterial*
{
	return m_material_.Get();
}

void cMeshComponent::SetMesh( const cShared_ptr< cAsset_Meta >& _mesh )
{
	m_mesh_.SetAsset( _mesh );
}

void cMeshComponent::SetMaterial( const cShared_ptr< cAsset_Meta >& _material )
{
	m_material_.SetAsset( _material );
}

 auto cMeshComponent::Serialize() -> cSerializedObject
 {
	cSerializedObject object( this );
	object.AddBase( iComponent::Serialize() );
	// TODO: Serialize asset refs properly
	object.WriteData( "mesh", m_mesh_.GetMeta() );
	object.WriteData( "material", m_material_.GetMeta() );
	object.EndWrite();
	return object;
 }
