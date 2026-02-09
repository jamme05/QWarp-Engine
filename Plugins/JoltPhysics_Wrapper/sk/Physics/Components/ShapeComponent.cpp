

#include "ShapeComponent.h"

#include <sk/Physics/Physics_Manager.h>
#include <sk/Physics/Components/RigidBodyComponent.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace sk::Physics::Components;

cShapeComponent::~cShapeComponent()
{
    if( !m_body_id_.IsInvalid() )
    {
        const auto& body_interface = cPhysics_Manager::get().m_physics_system_->GetBodyInterface();
        auto user_data = body_interface.GetUserData( m_body_id_ );
        auto& weak = reinterpret_cast< cWeak_Ptr< iComponent >& >( user_data );
        weak.~cWeak_Ptr();
    }
}

void cShapeComponent::enabled()
{

}

void cShapeComponent::disabled()
{

}

void cShapeComponent::registerSelf( const JPH::ShapeSettings& _shape )
{
    auto& body_interface = cPhysics_Manager::get().m_physics_system_->GetBodyInterface();

    const auto position = JPH::Vec3{ reinterpret_cast< const JPH::Float3& >( GetPosition() ) };
    const auto rotation = JPH::Mat44::sLoadFloat4x4( &reinterpret_cast< const JPH::Float4& >( GetWorld().x ) ).GetQuaternion();

    const JPH::BodyCreationSettings settings{ &_shape, position, rotation, JPH::EMotionType::Static, 0 };

    if( auto parent = m_parent_.Lock(); parent != nullptr && parent->getClassType() == kTypeId< cRigidBodyComponent > )
    {
        // TODO: Use rigidbody settings.
    }

    auto body = body_interface.CreateBody( settings );

    // TODO: Hande shape recreation
    // We want to keep the weak pointer valid as long as the body lives, so we need to detach it.
    auto weak = get_weak();
    body->SetUserData( reinterpret_cast< size_t& >( weak ) );
    memset( &weak, 0, sizeof( weak ) );



    cPhysics_Manager::get().addBody( body );
}
