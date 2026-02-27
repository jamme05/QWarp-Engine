//
// Created by willness on 2026-02-05.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//

#pragma once

#include <functional>
#include <map>
#include <stack>
#include <string_view>
#include <variant>
#include <vector>

namespace sk::Editor::Utils
{
    class cContextMenu
    {
        enum class eSetting
        {
            kSetDisabled,
            kSetEnabled,
        };
    public:
        struct sBranch;
        using predicate_t = std::function< bool( void* ) >;
        using selection_t = std::function< int_fast32_t( void* ) >;
        using callback_t  = std::function< void( void* ) >;

        struct sCustom
        {
            callback_t callback;
        };
        using value_t = std::variant< std::monostate, bool*, callback_t, sCustom, eSetting, std::unique_ptr< cContextMenu >, std::unique_ptr< sBranch > >;

        struct sItem
        {
            std::string name;
            value_t     value;
        };

        struct sBranch
        {
            using range_t     = std::pair< uint32_t, uint32_t >;
            using range_vec_t = std::vector< range_t >;
            using value_map_t = std::multimap< int_fast32_t, sItem >;
            selection_t  selection;
            int_fast32_t default_selection;
            value_map_t  value_map;
        };

        cContextMenu();
        cContextMenu( const cContextMenu& ) = default;
        cContextMenu( cContextMenu&& ) = default;
        ~cContextMenu();

        cContextMenu& operator=( const cContextMenu& ) = default;
        cContextMenu& operator=( cContextMenu&& ) = default;

        bool Draw( const std::string_view& _name = {} );
        bool DrawOnWindow( const std::string_view& _name = {} );

        // Adds a text item to the menu
        auto Add( const std::string& _text ) -> cContextMenu&;
        // Do note, the bool value has to survive as long as this context.
        // Adds a checkbox item to the menu
        auto Add( const std::string& _name, bool& _bool_value ) -> cContextMenu&;
        // Adds a button item to the menu
        auto Add( const std::string& _name, const callback_t& _callback ) -> cContextMenu&;
        auto Add( const std::string& _name, cContextMenu _menu ) -> cContextMenu&;
        // Inject a function to be ran during the rendering. Great for doing extra ImGui calls.
        auto AddCustom( const callback_t& _custom ) -> cContextMenu&;

        // Adds a button with the user data already cast. DO REMEMBER TO KEEP TRACK OF THE TYPE
        template< class Ty >
        auto Add( const std::string& _name, const std::function< void( Ty* ) >& _callback ) -> cContextMenu&
        {
            return Add( _name, [ _callback ]( void* _data ){ _callback( static_cast< Ty* >( _data ) ); } );
        }

        auto If    ( const predicate_t& _predicate ) -> cContextMenu&;
        template< class Ty >
        auto If( const std::function< bool( Ty* ) >& _predicate ) -> cContextMenu&
        {
            return If( [ _predicate ]( void* _data ){ return _predicate( static_cast< Ty* >( _data ) ); } );
        }
        auto ElseIf( const predicate_t& _predicate ) -> cContextMenu&;
        template< class Ty >
        auto ElseIf( const std::function< bool( Ty* ) >& _predicate ) -> cContextMenu&
        {
            return ElseIf( [ _predicate ]( void* _data ){ return _predicate( static_cast< Ty* >( _data ) ); } );
        }
        auto Else  () -> cContextMenu&;
        auto EndIf () -> cContextMenu&;

        auto Switch   ( const selection_t& _selection ) -> cContextMenu&;
        template< class Ty >
        auto Switch( const std::function< int_fast32_t( Ty* ) >& _selection ) -> cContextMenu&
        {
            return If( [ _selection ]( void* _data ){ return _selection( static_cast< Ty* >( _data ) ); } );
        }
        auto Case     ( uint_fast32_t _value ) -> cContextMenu&;
        auto Default  () -> cContextMenu&;
        auto EndSwitch() -> cContextMenu&;

        /**
         * You are not required to call Complete on the submenu.
         * @param _name The name of the submenu
         * @return The submenu to fill with content
         */
        auto AddSubMenu( const std::string& _name ) -> cContextMenu&;
        /**
         *
         * @return A reference to the parent menu
         */
        auto EndSubMenu() -> cContextMenu&;

        // Set the following items to either enabled or disabled
        // Note: You don't need to mark it as disabled in the end.
        auto SetDisabled( bool _is_disabled ) -> cContextMenu&;

        void Complete();

        void SetNextUserData( void* _data );

        bool IsValid() const;

    private:
        enum class eActiveBuilder
        {
            kNone,
            kSelf,
            kIf,
            kElse,
            kSwitch,
            kChild
        };
        struct sIfBuilder
        {
            using branches_t = std::vector< predicate_t >;
            using value_map_t = std::multimap< int_fast32_t, sItem >;
            size_t      active_if_index;
            branches_t  statements;
            value_map_t values_map;
        };
        struct sSwitchBuilder
        {
            using value_map_t = std::multimap< int_fast32_t, sItem >;
            using value_vec_t = std::vector< sItem >;
            int_fast32_t active_switch_case;
            bool         using_default_case;
            selection_t  selection;
            value_map_t  value_map;
            value_vec_t  default_values;
        };
        auto _getActiveBuilder() -> eActiveBuilder;
        void _drawItems();
        void _drawItem( sItem& _item );
        void _add( sItem&& _value );

        std::stack< eActiveBuilder >  m_active_builder_stack_;
        std::stack< sSwitchBuilder* > m_switch_builder_stack_;
        std::stack< sIfBuilder* >     m_if_builders_;

        cContextMenu* m_parent_menu_ = nullptr;
        void*         m_user_data_   = nullptr;
        std::vector< sItem > m_items_;

        bool m_currently_disabled_ = false;
    };
} // sk::Editor::Utils::