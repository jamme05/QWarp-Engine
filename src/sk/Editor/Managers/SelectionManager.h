//
// Created by willness on 2026-02-05.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#pragma once

#include <sk/Assets/Asset.h>
#include <sk/Containers/Map.h>
#include <sk/Misc/Singleton.h>
#include <sk/Misc/UUID.h>

#include "imgui.h"
#include "sk/Scene/Components/Component.h"

namespace sk::Object
{
    class cObject;
} // sk::Object::

namespace sk::Editor::Managers
{
    class cSelectionManager : public cSingleton< cSelectionManager >
    {
    public:
        cSelectionManager();

        [[ nodiscard ]] auto& GetObjects   () const { return m_selected_objects_;    }
        [[ nodiscard ]] auto& GetComponents() const { return m_selected_components_; }
        [[ nodiscard ]] auto& GetAssets    () const { return m_selected_assets_;    }

        bool IsSelected( const Object::cObject& _object );
        bool IsSelected( const Object::iComponent& _component ) const;
        bool IsSelected( const cAsset_Meta& _meta ) const;

        void BeginMultiSelection( ImGuiMultiSelectFlags _flags = ImGuiMultiSelectFlags_None );
        template< sk_class GroupBase >
        bool Selectable( const cWeak_Ptr< GroupBase >& _instance );
        bool Selectable( const type_info_t& _group_type, const cWeak_Ptr< iClass >& _instance );
        void EndMultiSelection();

        void Clear();
        void Clean();
    private:
        using object_map_t    = unordered_map< hash< cUUID >, ImGuiID >;
        using component_map_t = unordered_map< hash< cUUID >, ImGuiID >;
        using meta_map_t      = unordered_map< hash< cUUID >, ImGuiID >;
        using items_vec_t     = std::vector< std::pair< ImGuiID, cWeak_Ptr< iClass > > >;

        struct sSelectionGroup : ImGuiSelectionExternalStorage
        {
            sSelectionGroup();
            
            items_vec_t items;
        };

        ImGuiSelectionBasicStorage m_storage_;
        size_t m_current_item_idx_ = 0;
        size_t m_selection_group_id_   = 0;

        items_vec_t m_items_;

        size_t          m_expected_component_index_ = std::numeric_limits< size_t >::max();
        object_map_t    m_selected_objects_;
        component_map_t m_selected_components_;
        meta_map_t      m_selected_assets_;
    };

    template< sk_class Ty >
    bool cSelectionManager::Selectable( const cWeak_Ptr< Ty >& _instance )
    {
        return Selectable( kTypeInfo< Ty >, _instance );
    }
} // sk::Editor::Managers::