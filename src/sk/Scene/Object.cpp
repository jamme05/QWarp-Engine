/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Object.h"

#include <sk/Scene/Components/MeshComponent.h>
#include <sk/Scene/Managers/Layer_Manager.h>
#include <sk/Seralization/SerializedObject.h>

sk::Object::cObject::cObject( const std::string& _name )
: m_name( _name )
{
    m_root   = CreateComponent< Components::cTransformComponent >();
    SetLayer( 0 );
}

sk::Object::cObject::cObject( cSerializedObject& _object )
{
    _object.BeginRead( this );
    m_name = _object.ReadValueOr< std::string >( "name", "" );
    m_uuid_ = cUUID::FromString( _object.ReadValueOr< std::string >( "uuid", "" ) );
    if( m_uuid_ == cUUID::kInvalid )
        m_uuid_ = GenerateRandomUUID();
    m_root  = _object.ReadValue< cSerializedObject* >( "root" )->ConstructSharedClassAs< iComponent >();
    SetLayer( _object.ReadValueOr< uint64_t >( "layer", 0 ) );

    m_root->SetObject( get_weak() );

    _object.EndRead();
}

sk::Object::cObject::~cObject()
{
    if( const auto ptr = Scene::cLayer_Manager::getPtr() )
        ptr->RemoveObject( get_shared() );

    m_children_.clear();
    m_components_.clear();
    m_root = nullptr;
}

void sk::Object::cObject::AddComponent( const cShared_ptr< iComponent >& _component )
{
    _component->SetObject( get_weak() );

    if( _component->getClassTypeInfo() == kTypeInfo< Components::cMeshComponent > )
        m_mesh_components_.emplace_back( sk::cShared_ptr{ _component }.Cast< Components::cMeshComponent >() );
    else
	    m_components_.emplace( std::pair{ _component->getClassType(), _component } );
}

void sk::Object::cObject::RemoveComponent( const cShared_ptr< iComponent >& _component )
{
    if( _component->getClassTypeInfo() == kTypeInfo< Components::cMeshComponent > )
    {
        if( auto itr = std::ranges::find( m_mesh_components_, cShared_ptr{ _component }.Cast< Components::cMeshComponent >() ); itr != m_mesh_components_.end() )
            m_mesh_components_.erase( itr );
    }
    else
    {
        auto [ fst, lst ] = m_components_.equal_range( _component->getClassType() );
        for( ; fst != lst; ++fst )
        {
            if( fst->second == _component )
            {
                m_components_.erase( fst );
                return;
            }
        }
    }
}

void sk::Object::cObject::render()
{
    // TODO: Add actual event vector or something.
    for( auto& val : m_components_ | std::views::values )
    {
        val->PostEvent( kRender );
        val->PostEvent( kDebugRender );
    }
}

void sk::Object::cObject::update()
{
    for( auto& val : m_components_ | std::views::values )
        val->PostEvent( kUpdate );
}

void sk::Object::cObject::SetLayer( const uint64_t _layer )
{
    auto& layer_manager = Scene::cLayer_Manager::get();
    m_layer_ = _layer;
    layer_manager.AddObject( get_shared() );
}

auto sk::Object::cObject::Serialize() -> cSerializedObject
{
    return cSerializedObject{ this }
        .WriteValue( "name", m_name.string() )
        .WriteValue( "uuid", m_uuid_.ToString() )
        .WriteValue( "layer", m_layer_ )
        .WriteValue( "root", m_root->Serialize() )
    .EndWrite();
}

void sk::Object::cObject::SetRoot( const cShared_ptr< iComponent >& _new_root_component, const bool _override_parent )
{
    if( _override_parent )
        _new_root_component->SetParent( m_root->m_parent_.Lock() );
    else if( _new_root_component->m_parent_ == m_root )
        _new_root_component->SetParent( nullptr );

    while( !m_root->m_children_.empty() )
        m_root->m_children_.front()->SetParent( _new_root_component );
    
    m_root = _new_root_component;
}

void sk::Object::cObject::registerRecursive()
{
    for( auto& child : m_children_ )
        child->registerRecursive();

    m_root->registerRecursive();
}

void sk::Object::cObject::enableRecursive()
{
    for( auto& child : m_children_ )
        child->enableRecursive();

    m_root->enableRecursive();
}

void sk::Object::cObject::disableRecursive()
{
    for( auto& child : m_children_ )
        child->disableRecursive();

    m_root->disableRecursive();
}
