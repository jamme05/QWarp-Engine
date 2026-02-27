

#pragma once

#include <sk/Scene/Components/Component.h>
#include <sk/Scene/Components/Internal/Internal_Component.h>

namespace sk::Scene
{
    class cLayer_Manager;
} // sk::Scene

namespace sk::Object::Components
{
    SK_COMPONENT_CLASS( Layer_Info_Component, cInternal_Component )
    {
        SK_CLASS_BODY( Layer_Info_Component )
        friend class sk::Scene::cLayer_Manager;
        size_t m_layer_index_ = std::numeric_limits< size_t >::max();
        size_t m_index_       = std::numeric_limits< size_t >::max();
    };
} // sk::Object::Components

SK_DECLARE_CLASS( sk::Object::Components::Layer_Info_Component )