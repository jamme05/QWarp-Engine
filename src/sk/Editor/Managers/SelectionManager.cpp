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

bool cSelectionManager::IsSelected( const Object::cObject& _object )
{
    auto& obj = const_cast< Object::cObject& >( _object );
    auto [ _, component ]
        = obj.AddOrGetInternalComponent< Components::cEditorInternalComponent >( m_expected_component_index_ );

    return component->m_selected_;
}

bool cSelectionManager::IsSelected( const Object::iComponent& _component ) const
{
    const bool result = m_selected_components_.contains( _component.GetUUID() );
    if( result )
        sk::println( "Component {} (UUID: {}) Is Selected.", _component.getClass().getName(), _component.GetUUID().ToString() );

    return result;
}

bool cSelectionManager::IsSelected( const cAsset_Meta& _meta ) const
{
    return m_selected_assets_.contains( _meta.GetUUID() );
}

bool cSelectionManager::Selectable( const type_info_t& _group_type, const cWeak_Ptr< iClass >& _instance )
{
    m_current_item_idx_ = m_items_.size();
    const ImGuiSelectionUserData user_data = static_cast< ImGuiSelectionUserData >( m_current_item_idx_ );
    ImGui::SetNextItemSelectionUserData( user_data );
    const auto label = std::format( "##{}_{}", m_selection_group_id_, m_current_item_idx_ );
    m_items_.emplace_back( ImGui::GetID( label.c_str() ), _instance );
    ImGui::Selectable( label.c_str() );
}

void cSelectionManager::BeginMultiSelection( const ImGuiMultiSelectFlags _flags )
{
    const auto ms_io = ImGui::BeginMultiSelect( _flags, m_storage_.Size );
    m_storage_.ApplyRequests( ms_io );
}

void cSelectionManager::EndMultiSelection()
{
    ++m_selection_group_id_;
    const auto ms_io = ImGui::EndMultiSelect();
    m_storage_.ApplyRequests( ms_io );
}

void cSelectionManager::Clear()
{
    m_storage_.Clear();
    m_selected_objects_.clear();
    m_selected_components_.clear();
    m_selected_assets_.clear();
}

void cSelectionManager::Clean()
{
    // TODO: Redo
}

cSelectionManager::sSelectionGroup::sSelectionGroup()
{
    AdapterSetItemSelected = []( ImGuiSelectionExternalStorage* _self, int _idx, bool _selected )
    {

    };
}
