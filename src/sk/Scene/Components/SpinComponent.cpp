
#include "SpinComponent.h"

#include <sk/Platform/Time.h>

using namespace sk::Object::Components;

cSpinComponent::cSpinComponent( cVector3f _speed )
: m_speed_( std::move( _speed ) )
{}

cSpinComponent::cSpinComponent( cSerializedObject& _object )
: cComponent( _object.GetBase< iComponent >().value() )
{
    _object.BeginRead( this );
    m_speed_ = _object.ReadValueOr< cVector3f >( "speed", kZero );
    _object.EndRead();
}

void cSpinComponent::update()
{
    GetTransform().GetLocalRotation() += m_speed_ * Time::Delta;
    GetTransform().MarkDirty();

    for( auto& child : GetChildren() )
        child->GetTransform().MarkDirty();
}

auto cSpinComponent::Serialize() -> cSerializedObject
{
    cSerializedObject object( this );
    object.AddBase( cComponent::Serialize() );
    object.WriteValue( "speed", cVector3d{ m_speed_ } );
    object.EndWrite();
    return object;
}
