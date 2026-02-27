
#pragma once

#include <sk/Scene/Components/Component.h>
#include <sk/Scene/Components/Internal/Internal_Component.h>

#include <set>

namespace sk::Editor::Components
{
    SK_COMPONENT_CLASS( EditorInternalComponent, Object::Components::cInternal_Component )
    {
        SK_CLASS_BODY( EditorInternalComponent )

        friend class sk::Editor::Managers::cSelectionManager;
    public:
        bool IsSelected() const { return m_selected_; }
    private:
        bool              m_selected_ = false;
        std::set< cUUID > m_selected_components_;
    };
} // sk::Editor::Components::

SK_DECLARE_CLASS( sk::Editor::Components::EditorInternalComponent )