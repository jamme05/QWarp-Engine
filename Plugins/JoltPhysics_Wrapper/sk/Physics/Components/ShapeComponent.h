

#pragma once

#include <sk/Scene/Components/Component.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace JPH
{
    class ShapeSettings;
} // JPH

namespace sk::Physics::Components
{
    SK_COMPONENT_CLASS( ShapeComponent )
    {
        SK_CLASS_BODY( ShapeComponent )

    sk_public:
        ~cShapeComponent() override;
        void enabled () override;
        void disabled() override;

    protected:
        virtual void registerSelf( const JPH::ShapeSettings& _shape );

    private:
        JPH::BodyID m_body_id_;
    };
} // sk::Physics::Components::

SK_DECLARE_CLASS( sk::Physics::Components::ShapeComponent )