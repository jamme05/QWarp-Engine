//
// Created by willness on 2026-02-05.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//

#include "SelectionManager.h"

#include <sk/Editor/Components/EditorInternalComponent.h>
#include <sk/Scene/Object.h>

using namespace sk::Editor::Managers;

cSelectionManager::cSelectionManager()
{
    /*
    m_storage_.UserData = this;
    m_storage_.AdapterIndexToStorageId = []( ImGuiSelectionBasicStorage* _self, int _idx )
    {

    }*/
}

auto cSelectionManager::GetItemCollection( class_info_t& _group_class ) const -> const items_vec_t&
{
    if( const auto itr = m_groups_.find( _group_class.getTypeHash() ); itr != m_groups_.end() )
        return itr->second.items;
    SK_FATAL( "Error: No item collection for type: {}", _group_class.getRawName() )
}

auto cSelectionManager::GetSelectedCollection(class_info_t& _group_class) const -> const selected_map_t&
{
    if( const auto itr = m_groups_.find( _group_class.getTypeHash() ); itr != m_groups_.end() )
        return itr->second.selected;
    SK_FATAL( "Error: No item collection for type: {}", _group_class.getRawName() )
}

auto cSelectionManager::GetItem( class_info_t& _group_class, size_t _index ) const -> const sItem&
{
    return GetItemCollection( _group_class )[ _index ];
}

auto cSelectionManager::GetInstance( class_info_t& _group_class, size_t _index ) const -> const cWeak_Ptr< iClass >&
{
    return GetItem( _group_class, _index ).instance;
}

bool cSelectionManager::Selectable( class_info_t& _group_class, const cWeak_Ptr< iClass >& _instance )
{
    auto& group = m_groups_[ _group_class.getTypeHash() ];

    const auto current_item_idx = group.items.size();
    const auto user_data = static_cast< ImGuiSelectionUserData >( current_item_idx );
    ImGui::SetNextItemSelectionUserData( user_data );

    const auto label = std::format( "##{}_{}", _group_class.getRawName(), current_item_idx );
    group.items.emplace_back( ImGui::GetID( label.c_str() ), _instance );

    ImGui::Selectable( label.c_str() );
}

void cSelectionManager::BeginSelection( class_info_t& _group_class, const ImGuiMultiSelectFlags _flags )
{
    auto& group = m_groups_[ _group_class.getTypeHash() ];

    const auto ms_io = ImGui::BeginMultiSelect( _flags, group.items.size() );
    group.Apply( ms_io );
}

void cSelectionManager::EndSelection( class_info_t& _group_class )
{
    auto& group = m_groups_[ _group_class.getTypeHash() ];

    const auto ms_io = ImGui::EndMultiSelect();
    group.Apply( ms_io );
}

void cSelectionManager::Clear()
{
    m_groups_.clear();
}

cSelectionManager::sSelectionGroup::sSelectionGroup()
{
    AdapterSetItemSelected = []( ImGuiSelectionExternalStorage* _self, int _idx, bool _selected )
    {

    };
}

void cSelectionManager::sSelectionGroup::Apply( ImGuiMultiSelectIO* _ms_io )
{

}
