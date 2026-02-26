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

        using items_map_t    = std::unordered_map< uint64_t, sItem >;
        using id_vec_t       = std::vector< uint64_t >;
        using selected_map_t = std::set< uint64_t >;

        struct sSelectionGroup
        {
            sSelectionGroup();

            void Apply( ImGuiMultiSelectIO* _ms_io );
            void ClearSelected();

            class_info_t*  group_class;
            id_vec_t       ids;
            items_map_t    items;
            selected_map_t selected;
        };

        cSelectionManager();

        template< sk_class Ty >
        [[ nodiscard ]] auto GetItemCollection() const -> const items_map_t&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetSelectedCollection() const -> const selected_map_t&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetItem( size_t _index ) const -> const sItem&;
        template< sk_class Ty >
        [[ nodiscard ]] auto GetInstance( size_t _index ) const -> const cWeak_Ptr< Ty >&;

        [[ nodiscard ]] auto GetItemCollection    ( class_info_t& _group_class ) const -> const items_map_t&;
        [[ nodiscard ]] auto GetSelectedCollection( class_info_t& _group_class ) const -> const selected_map_t&;
        [[ nodiscard ]] auto GetItem              ( class_info_t& _group_class, size_t _index ) const -> const sItem&;
        [[ nodiscard ]] auto GetInstance          ( class_info_t& _group_class, size_t _index ) const -> const cWeak_Ptr< iClass >&;

        template< sk_class Ty >
        void BeginSelection( ImGuiMultiSelectFlags _flags = ImGuiMultiSelectFlags_None );

        void BeginSelection( class_info_t& _group_class, ImGuiMultiSelectFlags _flags = ImGuiMultiSelectFlags_None );
        void MarkForDeletion( bool _single = false );
        bool Selectable( uint64_t _identifier, const cWeak_Ptr< iClass >& _instance );
        void EndSelection();

        void Clear();
    private:
        auto _getGroup( class_info_t& _group_class ) const -> const sSelectionGroup&;

        using group_map_t = unordered_map< type_hash, sSelectionGroup >;

        group_map_t m_groups_;
        size_t      m_marked_for_deletion_start_;
        size_t      m_marked_for_deletion_end_;

        sSelectionGroup* m_active_group_ = nullptr;
    };

    template< sk_class Ty >
    auto cSelectionManager::GetItemCollection() const -> const items_map_t&
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
} // sk::Editor::Managers::

namespace sk::Editor::Selection
{
    static constexpr ImGuiMultiSelectFlags kDefaultListFlags = ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnEscape;
} // sk::Editor::Selection::