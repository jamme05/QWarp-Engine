
#pragma once

#include <sk/Math/Vector3.h>
#include <sk/Misc/Smart_Ptrs.h>

#include <vector>

namespace sk::Object
{
    class cObject;
} // sk::Object::

namespace sk::Physics::Utils
{
    struct sRayCastResult
    {
        cWeak_Ptr< Object::cObject >    hit_object;
        cWeak_Ptr< Object::iComponent > hit_component;
    };

    // Returns the closest hit.
    extern bool RayCast( sRayCastResult& _result, const cVector3f& _position, const cVector3f& _direction, float _distance = std::numeric_limits< float >::infinity() );
    // Returns all hits.
    extern bool RayCast( std::vector< sRayCastResult >& _result, const cVector3f& _position, const cVector3f& _direction, float _distance = std::numeric_limits< float >::infinity() );
} // sk::Physics::Utils::