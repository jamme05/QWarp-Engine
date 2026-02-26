

#include "Component.h"

#include <sk/Scene/Object.h>
#include <sk/Scene/Components/MeshComponent.h>

using namespace sk::Object;

iComponent::iComponent( cSerializedObject& _object )
{
    _object.BeginRead( this );
    // TODO: Remove this const cast.
    const_cast< cUUID& >( GetUUID() ) = cUUID::FromString( _object.ReadValue< std::string_view >( "UUID" ) );
    m_transform_ = sk::MakeShared< cTransform >( *_object.ReadValue< cSerializedObject* >( "Transform" ) );
    if( cSerializedObject* children; _object.TryReadValue( "children", children ) )
    {
        for( const auto arr = children->GetArray< cSerializedObject >(); auto& child : arr )
            child.ConstructSharedClass().Cast< iComponent >()->SetParent( get_shared() );
    }
    _object.EndRead();
}

iComponent::~iComponent()
{
    m_object_ = nullptr;
    m_children_.clear();
}

void iComponent::SetParent( const cShared_ptr< iComponent >& _component )
{
    // Already childed
    if( m_parent_ == _component )
        return;

    if( const auto& new_object = _component->m_object_; m_object_ != _component->m_object_ )
        SetObject( new_object );

    if( m_parent_ && !m_parent_.Lock()->m_children_.empty() )
    {
        if( const auto itr = std::ranges::find( m_parent_->m_children_, get_shared() ); itr != m_parent_->m_children_.end() )
            m_parent_->m_children_.erase( itr );
    }
    m_parent_ = _component;
    if( _component )
    {
        _component->m_children_.emplace_back( get_shared() );
        m_object_ = _component->m_object_;
    }
    m_transform_->SetParent( ( _component != nullptr ) ? _component->m_transform_ : nullptr );
}

auto iComponent::Serialize() -> cSerializedObject
{
    cSerializedObject object{ this };
    object.BeginWrite();
    object.WriteValue( "UUID", GetUUID().ToString() );
    object.WriteValue( "Transform", m_transform_->Serialize() );

    std::vector< cSerializedObject > children;
    for( auto& child : m_children_ )
    {
        if( !child->m_internal_ )
            children.emplace_back( std::move( child->Serialize() ) );
    }
    if( !children.empty() )
        object.WriteValue( "children", cSerializedObject::ConsumeArray( children.data(), children.size() ) );
    object.EndWrite();

    return object;
}

void iComponent::SetObject( const cWeak_Ptr< cObject >& _parent_object )
{
    for( auto& child : m_children_ )
        child->SetObject( _parent_object );

    if( m_object_.is_valid() )
        m_object_->RemoveComponent( get_shared() );
    m_object_ = _parent_object;
    m_scene_  = _parent_object->m_scene_;
    _parent_object->AddComponent( get_shared() );
}

void iComponent::registerRecursive()
{
    m_transform_->Update();
    for( auto& child : m_children_ )
        child->registerRecursive();
    registerEvents();
}

void iComponent::enableRecursive()
{
    for( auto& child : m_children_ )
    {
        if( child->m_enabled_ )
            child->enableRecursive();
    }
    enabled();
}

void iComponent::disableRecursive()
{
    for( auto& child : m_children_ )
        child->disableRecursive();
}
