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

auto cSelectionManager::GetItemCollection( class_info_t& _group_class ) const -> const items_map_t&
{
    if( const auto itr = m_groups_.find( _group_class.getTypeHash() ); itr != m_groups_.end() )
        return itr->second.items;
    SK_FATAL( "Error: No item collection for type: {}", _group_class.getRawName() )
}

auto cSelectionManager::GetSelectedCollection( class_info_t& _group_class ) const -> const selected_map_t&
{
    if( const auto itr = m_groups_.find( _group_class.getTypeHash() ); itr != m_groups_.end() )
        return itr->second.selected;
    SK_FATAL( "Error: No item collection for type: {}", _group_class.getRawName() )
}

auto cSelectionManager::GetItem( class_info_t& _group_class, size_t _index ) const -> const sItem&
{
    auto& group = _getGroup( _group_class );
    if( const auto itr = group.items.find( _index ); itr != group.items.end() )
        return itr->second;

    SK_FATAL( "This shouldn't happen." )
}

auto cSelectionManager::GetInstance( class_info_t& _group_class, size_t _index ) const -> const cWeak_Ptr< iClass >&
{
    return GetItem( _group_class, _index ).instance;
}


auto cSelectionManager::_getGroup( class_info_t& _group_class ) const -> const sSelectionGroup&
{
    if( const auto itr = m_groups_.find( _group_class.getTypeHash() ); itr != m_groups_.end() )
        return itr->second;

    SK_FATAL( "Group Does not exist." )
}

void cSelectionManager::BeginSelection( class_info_t& _group_class, const ImGuiMultiSelectFlags _flags )
{
    auto& group = m_groups_[ _group_class.getTypeHash() ];
    group.group_class = &_group_class;
    group.ids.clear();

    m_active_group_   = &group;

    m_marked_for_deletion_start_ = m_marked_for_deletion_end_ = std::numeric_limits< size_t >::max();

    const auto ms_io = ImGui::BeginMultiSelect( _flags, static_cast< int >( group.selected.size() ) );
    group.Apply( ms_io );
}

void cSelectionManager::MarkForDeletion( const bool _single )
{

}

bool cSelectionManager::Selectable( uint64_t _identifier, const cWeak_Ptr< iClass >& _instance )
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, m_active_group_ == nullptr, "Error: Can't end selection that hasn't been opened.", false );
    auto& group = *m_active_group_;

    const auto current_item_idx = group.ids.size();
    const auto user_data = static_cast< ImGuiSelectionUserData >( current_item_idx );
    ImGui::SetNextItemSelectionUserData( user_data );
    group.ids.emplace_back( _identifier );

    const bool selected = group.selected.contains( _identifier );

    const auto label = std::format( "##{}_{}", group.group_class->getRawName(), current_item_idx );
    group.items.emplace( _identifier, sItem{ ImGui::GetID( label.c_str() ), _instance } );

    // ImGui::Selectable( label.c_str(), selected );

    return selected;
}

void cSelectionManager::EndSelection()
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, m_active_group_ == nullptr, "Error: Can't end selection that hasn't been opened." );
    auto& group = *m_active_group_;

    const auto ms_io = ImGui::EndMultiSelect();
    group.Apply( ms_io );

    m_active_group_ = nullptr;
}

void cSelectionManager::Clear()
{
    m_groups_.clear();
}

cSelectionManager::sSelectionGroup::sSelectionGroup()
{

}

void cSelectionManager::sSelectionGroup::Apply( ImGuiMultiSelectIO* _ms_io )
{
    for( const auto& req : _ms_io->Requests )
    {
        if( req.Type == ImGuiSelectionRequestType_SetAll )
        {
            ClearSelected();
            // Maybe make everything being selected into a flag?
            if( req.Selected )
            {
                for( size_t i = 0; i < ids.size(); i++ )
                    selected.emplace( ids[ i ] );
            }
        }
        else if( req.Type == ImGuiSelectionRequestType_SetRange )
        {
            const auto last = static_cast< size_t >( req.RangeLastItem ) + 1;

            for( auto i = static_cast< size_t >( req.RangeFirstItem ); i < last; i++ )
            {
                if( req.Selected )
                    selected.emplace( ids[ i ] );
                else
                    selected.erase( ids[ i ] );
            }
        }
    }
}

void cSelectionManager::sSelectionGroup::ClearSelected()
{
    selected.clear();
}
