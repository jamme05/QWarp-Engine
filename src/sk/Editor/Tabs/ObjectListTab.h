
#pragma once

#include "Tab.h"

#include <sk/Editor/Utils/ContextMenu.h>
#include <sk/Scene/Object.h>

namespace sk
{
    namespace Object
    {
        class cObject;
    } // sk::Object::

    class cScene;
} // sk::

namespace sk::Editor::Tabs
{
    class cObjectListTab : public aTab
    {
    public:
        explicit cObjectListTab( const std::string& _name ) : aTab( _name ){}

        void Create () override;
        void Draw   () override;
        void Destroy() override;

    private:
        void _drawScene( const cScene& _scene );
        void _drawObjectRecursive( Object::cObject& _object );
        void _drawComponentsRecursive( Object::iComponent& _component );

        bool m_debug_view_      = false;
        bool m_show_components_ = true;
        Utils::cContextMenu m_context_menu_;
        Utils::cContextMenu m_scene_context_menu_;
        Utils::cContextMenu m_object_context_menu_;
        Utils::cContextMenu m_component_context_menu_;
    };
} // sk::Editor::Tabs::

