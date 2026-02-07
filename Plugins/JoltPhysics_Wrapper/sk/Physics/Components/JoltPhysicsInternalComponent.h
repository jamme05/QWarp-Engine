//
// Created by jag on 2026-02-07.
//

#pragma once

#include <sk/Scene/Components/Component.h>

namespace sk::Physics
{
    class cPhysics_Manager;
} // sk::Physics::

namespace sk::Physics::Components
{
    // TODO: Remove?
    SK_COMPONENT_CLASS( JoltPhysicsInternalComponent )
    {
        SK_CLASS_BODY( JoltPhysicsInternalComponent )
        friend class sk::Physics::cPhysics_Manager;

    private:

    };
} // sk::Physics::Components::

SK_DECLARE_CLASS( sk::Physics::Components::JoltPhysicsInternalComponent )