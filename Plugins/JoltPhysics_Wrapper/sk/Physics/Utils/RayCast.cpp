
#include "RayCast.h"

#include <sk/Physics/Physics_Manager.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

using namespace sk::Physics::Utils;

bool sk::Physics::Utils::RayCast( sRayCastResult& _result, const cVector3f& _position, const cVector3f& _direction, const float _distance )
{
    auto& physics_system = cPhysics_Manager::get().GetPhysicsSystem();
    auto& np_query = physics_system.GetNarrowPhaseQuery();

    JPH::RRayCast ray_cast{
        { _position.x, _position.y, _position.z },
        { _direction.x * _distance, _direction.y * _distance, _direction.z * _distance }
    };

    JPH::RayCastResult result;

    np_query.CastRay( ray_cast, result,  )
}

bool sk::Physics::Utils::RayCast( std::vector< sRayCastResult >& _result, const cVector3f& _position, const cVector3f& _direction, const float _distance )
{
    auto& physics_system = cPhysics_Manager::get().GetPhysicsSystem();
    auto& bp_query       = physics_system.GetBroadPhaseQuery();
}
