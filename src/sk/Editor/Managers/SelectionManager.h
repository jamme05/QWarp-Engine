//
// Created by willness on 2026-02-05.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#pragma once

#include <sk/Assets/Asset.h>
#include <sk/Containers/Map.h>
#include <sk/Misc/Singleton.h>
#include <sk/Misc/UUID.h>
#include <sk/Scene/Components/Component.h>

#include "imgui.h"

#include <set>

namespace sk::Object
{
    class cObject;
} // sk::Object::

namespace sk::Editor::Managers
{
    class cSelectionManager : public cSingleton< cSelectionManager >
    {
    public:
        struct sItem
        {
            ImGuiID             id;
            cWeak_Ptr< iClass > instance;
        };

        using items_vec_t    = std::vector< sItem >;
        using selected_map_t = std::set< int >;

        struct sSelectionGroup : ImGuiSelectionExternalStorage
        {
            sSelectionGroup();

            void Apply( ImGuiMultiSelectIO* _ms_io );

            items_vec_t    items;
            selected_map_t selected;
        };

        cSelectionManager();

        template< sk_class Ty >
        [[ nodiscard ]] auto GetItemCollection() const -> const items_vec_t&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetSelectedCollection() const -> const selected_map_t&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetItem( size_t _index ) const -> const sItem&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetInstance( size_t _index ) const -> const cWeak_Ptr< Ty >&;

        [[ nodiscard ]] auto GetItemCollection    ( class_info_t& _group_class ) const -> const items_vec_t&;
        [[ nodiscard ]] auto GetSelectedCollection( class_info_t& _group_class ) const -> const selected_map_t&;
        [[ nodiscard ]] auto GetItem              ( class_info_t& _group_class, size_t _index ) const -> const sItem&;
        [[ nodiscard ]] auto GetInstance          ( class_info_t& _group_class, size_t _index ) const -> const cWeak_Ptr< iClass >&;

        template< sk_class Ty >
        void BeginSelection( ImGuiMultiSelectFlags _flags = ImGuiMultiSelectFlags_None );
        template< sk_class GroupBase >
        bool Selectable( const cWeak_Ptr< GroupBase >& _instance );
        template< sk_class GroupBase >
        void EndSelection();

        void BeginSelection( class_info_t& _group_class, ImGuiMultiSelectFlags _flags = ImGuiMultiSelectFlags_None );
        bool Selectable( class_info_t& _group_class, const cWeak_Ptr< iClass >& _instance );
        void EndSelection( class_info_t& _group_class );

        void Clear();
    private:

        using group_map_t = unordered_map< type_hash, sSelectionGroup >;

        group_map_t m_groups_;

        size_t m_expected_component_index_ = std::numeric_limits< size_t >::max();
    };

    template< sk_class Ty >
    auto cSelectionManager::GetItemCollection() const -> const items_vec_t&
    {
        return GetItemCollection( Ty::getStaticClass() );
    }

    template< sk_class Ty >
    auto cSelectionManager::GetSelectedCollection() const -> const selected_map_t&
    {
        return GetSelectedCollection( Ty::getStaticClass() );
    }

    template< sk_class Ty >
    auto cSelectionManager::GetItem( const size_t _index ) const -> const sItem&
    {
        return GetItem( Ty::getStaticClass(), _index );
    }

    template< sk_class Ty >
    auto cSelectionManager::GetInstance( const size_t _index ) const -> const cWeak_Ptr< Ty >&
    {
        return reinterpret_cast< const cWeak_Ptr< Ty >& >( GetInstance( Ty::getStaticClass(), _index ) );
    }

    template< sk_class Ty >
    void cSelectionManager::BeginSelection( ImGuiMultiSelectFlags _flags )
    {
        BeginSelection( Ty::getStaticClass(), _flags );
    }

    template< sk_class Ty >
    bool cSelectionManager::Selectable( const cWeak_Ptr< Ty >& _instance )
    {
        return Selectable( Ty::getStaticClass(), _instance );
    }

    template< sk_class GroupBase >
    void cSelectionManager::EndSelection()
    {
        EndSelection( GroupBase::getStaticClass() );
    }
} // sk::Editor::Managers::