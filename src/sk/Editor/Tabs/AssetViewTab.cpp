//
// Created by willness on 2026-02-06.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#include "AssetViewTab.h"

#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Editor/Managers/SelectionManager.h>
#include <sk/Scene/Scene.h>
#include <sk/Scene/Managers/SceneManager.h>

#include <imgui.h>


using namespace sk::Editor::Tabs;

namespace
{
    struct sAsset
    {
        bool _is_folder;
        sk::cAsset_Meta* meta;
    };
} // ::

void cAssetGridViewTab::Create()
{
    m_current_folder_ = std::filesystem::current_path();
    m_tab_context_menu_
        .If( [ current_folder=&m_current_folder_ ]( void* )
        {
            return *current_folder == std::filesystem::current_path();
        } )
            .SetDisabled( true )
        .EndIf()
        .Add( "Back", [ current_folder=&m_current_folder_ ]( void* )
        {
            if( *current_folder != std::filesystem::current_path() )
                *current_folder = current_folder->parent_path();
        } )
    .Complete();

    m_window_context_menu_
        .Add( "Create Scene", [ current_folder=&m_current_folder_ ]( void* )
        {
            cAsset_Manager::get().CreateAsset< cScene >( "Scene", ( *current_folder ) / "scene.skscene" );
        } )
    .Complete();
}

void cAssetGridViewTab::Draw()
{
    struct sFolder
    {
        std::string           name;
        std::filesystem::path path;
    };

    if( !m_tab_context_menu_.Draw() )
        m_window_context_menu_.DrawOnWindow( "asset_grid_popup" );

    // TODO: Redo all of this
    auto& assets = cAsset_Manager::get().GetAllAssets();
    std::vector< cShared_ptr< cAsset_Meta > > assets_in_folder;
    std::vector< sFolder > folders_in_folder;

    for( auto& entry : std::filesystem::directory_iterator{ m_current_folder_ } )
    {
        if( entry.is_directory() )
            folders_in_folder.emplace_back( sFolder{ entry.path().filename().string(), entry.path() } );
    }
    for( const auto& asset : assets | std::views::values )
    {
        if( std::filesystem::path{ asset->GetAbsolutePath().view() }.parent_path() == m_current_folder_ )
            assets_in_folder.emplace_back( asset );
    }

    const auto region = ImGui::GetContentRegionAvail();

    const auto width = region.x / 6;
    auto size  = ImVec2{ width - 5, width - 5 };

    // auto total_items = assets_in_folder.size() + folders_in_folder.size();

    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 } );

    size_t index = 0;
    for( auto& [ name, path ] : folders_in_folder )
    {
        if( ( index++ % 6 ) != 0 )
            ImGui::SameLine();

        if( ImGui::Button( name.c_str(), size ) )
            m_current_folder_ = path;
    }

    auto& selection_manager = Managers::cSelectionManager::get();

    for( auto& asset : assets_in_folder )
    {
        if( ( index++ % 6 ) != 0 )
            ImGui::SameLine();

        if( ImGui::Button( std::format( "{}##Asset_{}", asset->GetName().view(), index ).c_str(), size ) )
        {
            if( asset->GetType() == kTypeInfo< cScene > )
            {
                for( auto scenes = cSceneManager::get().GetScenes();
                    auto& scene_meta : scenes | std::views::values )
                {
                    if( asset->GetUUID() != scene_meta->GetUUID() )
                        cSceneManager::get().UnregisterScene( scene_meta->GetUUID() );
                }

                cSceneManager::get().RegisterScene( asset );
                cSceneManager::get().LoadScene( asset->GetUUID() );
            }
            println( "Asset {} selected", asset->GetName().c_str() );
        }
    }

    // WHY DID I DO THIS??
    // if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) )
    //     selection_manager.Clear();

    ImGui::PopStyleVar();
}

void cAssetGridViewTab::Destroy()
{

}
