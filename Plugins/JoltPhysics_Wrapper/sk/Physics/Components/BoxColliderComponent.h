

#pragma once

#include <sk/Scene/Components/Component.h>
#include <sk/Physics/Components/ShapeComponent.h>

namespace sk::Physics::Components
{
    SK_COMPONENT_CLASS( BoxColliderComponent, cShapeComponent )
    {
        SK_CLASS_BODY( BoxColliderComponent )
    };
} // sk::Physics::Components::

SK_DECLARE_CLASS( sk::Physics::Components::BoxColliderComponent )