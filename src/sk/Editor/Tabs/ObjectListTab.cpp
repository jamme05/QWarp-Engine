
#include "ObjectListTab.h"

#include <sk/Editor/Managers/SelectionManager.h>
#include <sk/Editor/Utils/ContextMenu.h>
#include <sk/Scene/Scene.h>
#include <sk/Scene/Managers/SceneManager.h>

#include <imgui.h>



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
    .Complete();
}

void cObjectListTab::Draw()
{
    const auto& manager = cSceneManager::get();

    m_context_menu_.Draw();

    for( auto scenes = manager.GetScenes();
        auto& scene_meta : scenes | std::views::values )
    {
        if( scene_meta->IsLoaded() )
            _drawScene( static_cast< cScene& >( *scene_meta->GetAsset() ) ); // NOLINT(*-pro-type-static-cast-downcast)
        else
        {
            ImGui::CollapsingHeader( scene_meta->GetName().c_str(), ImGuiTreeNodeFlags_Leaf );
            m_scene_context_menu_.SetUserData( scene_meta.get() );
            m_scene_context_menu_.Draw();
        }
    }
}

void cObjectListTab::Destroy()
{

}

void cObjectListTab::_drawScene( const cScene& _scene )
{
    auto& meta = *_scene.GetMeta();
    auto& selection_manager = Managers::cSelectionManager::get();

    selection_manager.BeginSelection< Object::cSceneItem >( ImGuiMultiSelectFlags_BoxSelect1d );

    if( ImGui::CollapsingHeader( meta.GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen ) )
    {
        m_scene_context_menu_.SetUserData( &meta );
        m_scene_context_menu_.Draw();


        for( auto& object : _scene.GetObjects() )
            _drawObjectRecursive( *object );
    }

    selection_manager.EndSelection< Object::cSceneItem >();
}

void cObjectListTab::_drawObjectRecursive( const Object::cObject& _object )
{
    auto& selection_manager = Managers::cSelectionManager::get();

    auto& children = _object.GetChildren();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_DefaultOpen;

    if( children.empty() && !m_show_components_ )
        flags |= ImGuiTreeNodeFlags_Leaf;

    auto id = ImGui::GetID( _object.GetUUID().ToString().c_str() );

    if( selection_manager.Selectable< Object::cSceneItem >( _object.get_weak() ) )
        flags |= ImGuiTreeNodeFlags_Selected;

    // selection_manager.Selectable( cUUID::kInvalid, _object.get_weak() );

    const bool opened = ImGui::TreeNodeEx( _object.GetUUID().ToString().c_str(), flags, "%s", _object.GetName().c_str() );

    if( !opened )
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

void cObjectListTab::_drawComponentsRecursive( const Object::iComponent& _component )
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

    if( selection_manager.Selectable< Object::cSceneItem >( _component.get_weak() ) )
        flags |= ImGuiTreeNodeFlags_Selected;

    const bool opened = ImGui::TreeNodeEx( _component.GetUUID().ToString().c_str(), flags, "%s", type_name.c_str() );

    if( !opened )
        return;

    for( auto& child : children )
        _drawComponentsRecursive( *child );

    ImGui::TreePop();
}
