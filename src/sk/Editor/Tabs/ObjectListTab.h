
#pragma once

#include "Tab.h"

#include <sk/Editor/Utils/ContextMenu.h>
#include <sk/Scene/Object.h>

struct ImGuiWindow;

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

        enum class eCreationType : uint8_t
        {
            kNone,
            kObject,
            kComponent
        };

        void OpenCreationPopup( eCreationType _type, const cWeak_Ptr< Object::cSceneItem >& _target );
        void DrawCreationPopup();

    private:
        void _drawScene( cScene& _scene );
        void _drawObjectRecursive( Object::cObject& _object );
        void _drawComponentsRecursive( Object::iComponent& _component );

        bool m_debug_view_      = false;
        bool m_show_components_ = true;
        eCreationType m_creation_type_ = eCreationType::kNone;
        cWeak_Ptr< Object::cSceneItem > m_creation_target_ = nullptr;
        cScene*             m_creation_target_scene_ = nullptr;
        ImGuiWindow*        m_root_window_ = nullptr;
        std::vector< class_info_t* > m_derived_types_cache_;

        Utils::cContextMenu m_context_menu_;
        Utils::cContextMenu m_scene_context_menu_;
        Utils::cContextMenu m_object_context_menu_;
        Utils::cContextMenu m_component_context_menu_;
    };
} // sk::Editor::Tabs::

