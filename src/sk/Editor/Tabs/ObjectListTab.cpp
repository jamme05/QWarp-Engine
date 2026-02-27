
#include "ObjectListTab.h"

#include <sk/Editor/Managers/SelectionManager.h>
#include <sk/Editor/Utils/ContextMenu.h>
#include <sk/Scene/Scene.h>
#include <sk/Scene/Managers/SceneManager.h>
#include <sk/Scene/Components/Internal/Internal_Component.h>

#include <imgui.h>
#include <imgui_internal.h>

using namespace sk::Editor::Tabs;

void cObjectListTab::Create()
{
    m_context_menu_
        .AddSubMenu( "Settings" )
            .Add( "Debug Mode", m_debug_view_ )
            .Add( "Show Components", m_show_components_ )
        .EndSubMenu()
    .Complete();

    m_scene_context_menu_
        .If< cAsset_Meta >( []( const cAsset_Meta* _meta )
        {
            return cSceneManager::get().GetLoadedScenes().contains( _meta->GetUUID() );
        } )
            .Add< cAsset_Meta >( "Unload", []( const cAsset_Meta* _meta )
            {
                cSceneManager::get().UnloadScene( _meta->GetUUID() );
            } )
        .Else()
            .Add< cAsset_Meta >( "Load", []( const cAsset_Meta* _meta )
            {
                const auto& meta = *_meta;
                cSceneManager::get().LoadScene( meta.GetUUID() );
            } )
        .EndIf()
        .Add< cAsset_Meta >( "Remove", []( const cAsset_Meta* _meta )
        {
            const auto& meta = *_meta;
            cSceneManager::get().UnregisterScene( meta.GetUUID() );
        } )
        .Add< cScene >( "Create Object", [this]( cScene* _scene )
        {
            ImGui::CloseCurrentPopup();
            this->m_creation_target_scene_ = _scene;
            this->OpenCreationPopup( eCreationType::kObject, nullptr );
        } )
    .Complete();

    m_object_context_menu_
        .Add< Object::cObject >( "Create Child", [this]( Object::cObject* _object )
        {
            ImGui::CloseCurrentPopup();
            this->OpenCreationPopup( eCreationType::kObject, _object->get_weak() );
        } )
    .Complete();

    m_component_context_menu_
        .Add< Object::iComponent >( "Create Child", [this]( Object::iComponent* _component )
        {
            ImGui::CloseCurrentPopup();
            this->OpenCreationPopup( eCreationType::kComponent, _component->get_weak() );
        } )
    .Complete();
}

void cObjectListTab::Draw()
{
    const auto& manager = cSceneManager::get();

    m_root_window_ = GImGui->CurrentWindow;

    m_context_menu_.Draw();

    for( auto scenes = manager.GetScenes();
        auto& scene_meta : scenes | std::views::values )
    {
        // TODO: The handle the scene still being marked as loaded when it's being destroyed.
        if( scene_meta->IsLoaded() )
        {
            scene_meta->LockAsset();
            _drawScene( static_cast< cScene& >( *scene_meta->GetAsset() ) ); // NOLINT(*-pro-type-static-cast-downcast)
            scene_meta->UnlockAsset();
        }
        else
        {
            ImGui::CollapsingHeader( scene_meta->GetName().c_str(), ImGuiTreeNodeFlags_Leaf );
            m_scene_context_menu_.SetNextUserData( scene_meta.get() );
            m_scene_context_menu_.Draw();
        }
    }

    DrawCreationPopup();
}

void cObjectListTab::Destroy()
{

}

void cObjectListTab::OpenCreationPopup( const eCreationType _type, const cWeak_Ptr< Object::cSceneItem >& _target )
{
    m_creation_type_   = _type;
    m_creation_target_ = _target;
    if( _target )
        m_creation_target_scene_ = &_target->GetScene();
    m_root_window_->IDStack.push_back( m_root_window_->IDStack.front() );
    ImGui::OpenPopupEx( m_root_window_->GetID( _type == eCreationType::kObject ? "Object Creation" : "Component Creation" ) );
    m_root_window_->IDStack.pop_back();
}

void cObjectListTab::DrawCreationPopup()
{
    if( m_creation_type_ == eCreationType::kNone )
        return;

    const bool is_object = m_creation_type_ == eCreationType::kObject;

    bool open = true;
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings;

    if( ImGui::BeginPopupModal( is_object ? "Object Creation" : "Component Creation", &open, flags ) )
    {
        if( ImGui::IsKeyPressed( ImGuiKey_Escape ) )
            ImGui::CloseCurrentPopup();

        if( m_derived_types_cache_.empty() )
        {
            static constexpr class_info_t& component_class = Object::iComponent::getStaticClass();
            static constexpr class_info_t& object_class    = Object::cObject::getStaticClass();

            auto& type_manager = Reflection::cType_Manager::get();
            m_derived_types_cache_ = type_manager.GetDerivedTypes( is_object ? object_class : component_class );
            if( !is_object )
            {
                std::erase_if( m_derived_types_cache_, []( class_info_t*& _type )
                {
                    return _type->isDerivedFrom( Object::Components::cInternal_Component::getStaticClass() );
                } );
            }
        }

        ImGui::Text( m_creation_type_ == eCreationType::kObject ? "Objet" : "Component" );
        for( const auto& available_class : m_derived_types_cache_ )
        {
            ImGui::BeginDisabled( !available_class->IsDefaultConstructible() );

            if( ImGui::Button( available_class->getRawName() ) )
            {
                ImGui::CloseCurrentPopup();

                if( is_object )
                {
                    auto& scene = *m_creation_target_scene_;
                    scene.AddObject( available_class->CreateDefaultShared().Cast< Object::cObject >() );

                    if( auto object = m_creation_target_.DynCast< Object::cObject >() )
                    {
                        // TODO: Add object as child to other object.
                    }
                }
                else if( m_creation_target_ ) // Verify
                {
                    // Component parent is
                    auto new_component = available_class->CreateDefaultShared().Cast< Object::iComponent >();

                    if( auto object = m_creation_target_.DynCast< Object::cObject >() )
                    {
                        object->AddComponent( new_component );
                        new_component->SetParent( object->GetRoot() );
                    }
                    else if( auto component = m_creation_target_.DynCast< Object::iComponent >() )
                    {
                        object = component->GetObject();
                        object->AddComponent( new_component );
                        new_component->SetParent( component.Lock() );
                    }
                }

                ImGui::EndDisabled();

                // We don't need to try the rest.
                break;
            }

            ImGui::EndDisabled();
        }
        ImGui::EndPopup();
    }
    else
    {
        m_creation_type_   = eCreationType::kNone;
        m_creation_target_ = nullptr;
        m_derived_types_cache_.clear();
    }
}

void cObjectListTab::_drawScene( cScene& _scene )
{
    auto& meta = *_scene.GetMeta();
    auto& selection_manager = Managers::cSelectionManager::get();

    selection_manager.BeginSelection< Object::cSceneItem >( Selection::kDefaultListFlags );

    if( ImGui::CollapsingHeader( meta.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        m_scene_context_menu_.SetNextUserData( &_scene );
        m_scene_context_menu_.Draw();

        for( auto& object : _scene.GetObjects() )
            _drawObjectRecursive( *object );
    }

    selection_manager.EndSelection();
}

void cObjectListTab::_drawObjectRecursive( Object::cObject& _object )
{
    auto& selection_manager = Managers::cSelectionManager::get();

    auto& children = _object.GetChildren();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;

    if( children.empty() && !m_show_components_ )
        flags |= ImGuiTreeNodeFlags_Leaf;

    if( selection_manager.Selectable( _object.GetUUID().Hash(), _object.get_weak() ) )
    {
        flags |= ImGuiTreeNodeFlags_Selected;

        if( ImGui::IsKeyDown( ImGuiKey_Delete ) )
            _object.Destroy();
    }

    const bool open = ImGui::TreeNodeEx( _object.GetUUID().ToString().c_str(), flags, "%s", _object.GetName().c_str() );

    m_object_context_menu_.SetNextUserData( &_object );
    m_object_context_menu_.Draw();

    if( !open )
        return;

    for( auto& object : children )
        _drawObjectRecursive( *object );

    // TODO: Draw this node slightly differently to show that this is showing something else.
    if( m_show_components_ && ImGui::TreeNode( ( "C_" + _object.GetUUID().ToString() ).c_str(), "Components" ) )
    {
        _drawComponentsRecursive( *_object.GetRoot() );

        ImGui::TreePop();
    }

    ImGui::TreePop();
}

void cObjectListTab::_drawComponentsRecursive( Object::iComponent& _component )
{
    // TODO: Allow the user to toggle the visual
    if( !m_debug_view_ && _component.GetIsInternal() )
        return;

    auto& selection_manager = Managers::cSelectionManager::get();

    const auto type_name = _component.getClass().getName();

    auto& children = _component.GetChildren();
    // TODO: Handle the case when debug view is enabled
    const auto has_non_internal_child = std::ranges::any_of( children, []( const auto& _child ){ return !_child->GetIsInternal(); } );

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= has_non_internal_child ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf;

    if( selection_manager.Selectable( _component.GetUUID().Hash(), _component.get_weak() ) )
    {
        flags |= ImGuiTreeNodeFlags_Selected;

        if( !_component.GetIsInternal() && ImGui::IsKeyDown( ImGuiKey_Delete ) )
            _component.Destroy();
    }

    const bool open = ImGui::TreeNodeEx( _component.GetUUID().ToString().c_str(), flags, "%s", type_name.c_str() );

    m_component_context_menu_.SetNextUserData( &_component );
    m_component_context_menu_.Draw();

    if( !open )
        return;

    for( auto& child : children )
        _drawComponentsRecursive( *child );

    ImGui::TreePop();
}
