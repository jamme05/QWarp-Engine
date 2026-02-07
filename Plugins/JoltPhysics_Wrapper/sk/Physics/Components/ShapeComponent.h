

#pragma once

#include <sk/Scene/Components/Component.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace sk::Physics::Components
{
    SK_COMPONENT_CLASS( ShapeComponent )
    {
        SK_CLASS_BODY( ShapeComponent )

    sk_public:
        void enabled () override;
        void disabled() override;

    protected:
        virtual void registerSelf( const JPH::ShapeSettings::ShapeResult& _shape );
    };
} // sk::Physics::Components::

SK_DECLARE_CLASS( sk::Physics::Components::ShapeComponent )