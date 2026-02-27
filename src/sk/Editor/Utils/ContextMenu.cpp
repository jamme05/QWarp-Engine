//
// Created by willness on 2026-02-05.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#include "ContextMenu.h"

#include <sk/Debugging/Macros/Assert.h>

#include "imgui_internal.h"

#include <imgui.h>
#include <ranges>

using namespace sk::Editor::Utils;

namespace
{
    // From: https://en.cppreference.com/w/cpp/utility/variant/visit
    template< class... Ts >
    struct sVisitor : Ts... { using Ts::operator()...; };
} // ::

cContextMenu::cContextMenu()
{
    m_active_builder_stack_.emplace( eActiveBuilder::kSelf );
}

cContextMenu::~cContextMenu() = default;

bool cContextMenu::Draw( const std::string_view& _name )
{
    if( m_parent_menu_ == nullptr )
    {
        // ImGuiContext& g = *GImGui;
        // ImGuiWindow* window = g.CurrentWindow;
        // ImGuiID parent_id = window ? window->IDStack.back() : g.LastItemData.ID;

        // Root menu/Popup
        if( ImGui::BeginPopupContextItem( _name.data() ) )
        {
            // g.CurrentWindow->IDStack.push_back( parent_id );
            _drawItems();
            // g.CurrentWindow->IDStack.pop_back();
            ImGui::EndPopup();
            m_user_data_ = nullptr;
            return true;
        }
    }
    else if( ImGui::BeginMenu( _name.data() ) )
    {
        // Sub menu
        _drawItems();
        ImGui::EndMenu();
        m_user_data_ = nullptr;
        return true;
    }
    m_user_data_ = nullptr;
    return false;
}

bool cContextMenu::DrawOnWindow( const std::string_view& _name )
{
    // Logic copied from: ImGui::BeginPopupContextItem
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if( window->SkipItems )
        return false;

    ImGuiID id = _name.empty() ? g.LastItemData.ID : window->GetID( _name.data() );

    if( ImGui::IsMouseReleased( ImGuiMouseButton_Right ) && ImGui::IsWindowHovered() )
        ImGui::OpenPopupEx( id );

    if( ImGui::BeginPopupEx( id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings ) )
    {
        _drawItems();
        ImGui::EndPopup();
        return true;
    }
    return false;
}

auto cContextMenu::Add( const std::string& _text ) -> cContextMenu&
{
    _add( sItem{ _text, std::monostate{} } );

    return *this;
}

auto cContextMenu::Add( const std::string& _name, bool& _bool_value ) -> cContextMenu&
{
    _add( sItem{ _name, &_bool_value } );

    return *this;
}

auto cContextMenu::Add( const std::string& _name, const callback_t& _callback ) -> cContextMenu&
{
    _add( sItem{ _name, _callback } );

    return *this;
}

auto cContextMenu::Add( const std::string& _name, cContextMenu _menu ) -> cContextMenu&
{
    auto menu = std::make_unique< cContextMenu >( std::move( _menu ) );
    menu->m_parent_menu_ = this;

    _add( sItem{ _name, std::move( menu ) } );

    return *this;
}

auto cContextMenu::AddCustom( const callback_t& _custom ) -> cContextMenu&
{
    _add( sItem{ {}, sCustom{ _custom } } );

    return *this;
}

auto cContextMenu::If( const predicate_t& _predicate ) -> cContextMenu&
{
    auto builder = new sIfBuilder{};
    m_if_builders_.emplace( builder );
    builder->statements.emplace_back( _predicate );

    m_active_builder_stack_.emplace( eActiveBuilder::kIf );

    return *this;
}

auto cContextMenu::ElseIf( const predicate_t& _predicate ) -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() == eActiveBuilder::kElse,
        "Error: You're not allowed to add more branches after else.", *this )

    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kIf,
        "Error: Context menu isn't currently building an if statement.", *this )

    auto& builder = *m_if_builders_.top();
    builder.active_if_index++;
    builder.statements.emplace_back( _predicate );

    return *this;
}

auto cContextMenu::Else() -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() == eActiveBuilder::kElse,
        "Error: You're not allowed to add more branches after else.", *this )

    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kIf,
        "Error: Context menu isn't currently building an if statement.", *this )

    m_if_builders_.top()->active_if_index = -1;

    return *this;
}

auto cContextMenu::EndIf() -> cContextMenu&
{
    SK_BREAK_RET_IFN( sk::Severity::kEditor, _getActiveBuilder() == eActiveBuilder::kIf || _getActiveBuilder() == eActiveBuilder::kElse,
        "Error: Context menu isn't currently building an if statement.", *this )

    auto& builder = *m_if_builders_.top();

    auto  branch_ptr = std::make_unique< sBranch >();
    auto& branch  = *branch_ptr;
    branch.selection = [ statements = builder.statements ]( void* _user_data )
    {
        for( size_t i = 0; i < statements.size(); i++ )
        {
            if( statements[ i ]( _user_data ) )
                return static_cast< int_fast32_t >( i );
        }
        return -1;
    };
    branch.default_selection = -1;
    branch.value_map.swap( builder.values_map );

    delete &builder;
    m_active_builder_stack_.pop();
    m_if_builders_.pop();

    _add( { {}, std::move( branch_ptr ) } );

    return *this;
}

auto cContextMenu::Switch( const selection_t& _selection ) -> cContextMenu&
{
    auto builder = new sSwitchBuilder{};
    m_switch_builder_stack_.emplace( builder );
    builder->selection = _selection;

    m_active_builder_stack_.emplace( eActiveBuilder::kSwitch );

    return *this;
}

auto cContextMenu::Case( uint_fast32_t _value ) -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kSwitch,
        "Error: Context menu isn't currently building a switch statement.", *this )

    auto& builder = *m_switch_builder_stack_.top();

    builder.active_switch_case = _value;
    builder.using_default_case = false;

    return *this;
}

auto cContextMenu::Default() -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kSwitch,
        "Error: Context menu isn't currently building a switch statement.", *this )

    auto& builder = *m_switch_builder_stack_.top();
    builder.using_default_case = true;

    return *this;
}

auto cContextMenu::EndSwitch() -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kSwitch,
        "Error: Context menu isn't currently building a switch statement.", *this )

    auto& builder = *m_switch_builder_stack_.top();
    auto  branch_ptr = std::make_unique< sBranch >();
    auto& branch  = *branch_ptr;

    auto default_index = ( --builder.value_map.end() )->first + 1;
    while( builder.value_map.contains( default_index ) )
        default_index++;

    branch.selection = builder.selection;
    branch.default_selection = default_index;
    branch.value_map.swap( builder.value_map );
    for( auto& value : builder.default_values )
        branch.value_map.emplace( default_index, std::move( value ) );

    _add( { {}, std::move( branch_ptr ) } );

    delete &builder;
    m_switch_builder_stack_.pop();
    m_active_builder_stack_.pop();

    return *this;
}

auto cContextMenu::AddSubMenu( const std::string& _name ) -> cContextMenu&
{
    auto temp_menu = std::make_unique< cContextMenu >();
    temp_menu->m_parent_menu_ = this;

    const auto ptr = temp_menu.get();

    _add( { _name, std::move( temp_menu ) } );

    m_active_builder_stack_.emplace( eActiveBuilder::kChild );

    return *ptr;
}

auto cContextMenu::EndSubMenu() -> cContextMenu&
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, _getActiveBuilder() != eActiveBuilder::kSelf && m_parent_menu_ != nullptr,
        "Error: This context menu isn't valid for this call.", *this )

    Complete();

    m_parent_menu_->m_active_builder_stack_.pop();

    return *m_parent_menu_;
}

auto cContextMenu::SetDisabled( bool _is_disabled ) -> cContextMenu&
{
    _add( { {}, _is_disabled ? eSetting::kSetDisabled : eSetting::kSetEnabled } );

    return *this;
}

void cContextMenu::Complete()
{
    SK_BREAK_IF( sk::Severity::kEngine, m_active_builder_stack_.top() != eActiveBuilder::kSelf,
        "Error: " )

    m_active_builder_stack_.pop();
}

void cContextMenu::SetNextUserData( void* _data )
{
    SK_BREAK_RET_IF( sk::Severity::kEditor, !IsValid(), "Error: Context menu isn't properly initialized." )

    m_user_data_ = _data;
}

bool cContextMenu::IsValid() const
{
    return m_active_builder_stack_.empty();
}

auto cContextMenu::_getActiveBuilder() -> eActiveBuilder
{
    return m_active_builder_stack_.empty() ? eActiveBuilder::kNone : m_active_builder_stack_.top();
}

void cContextMenu::_drawItems()
{
    SK_BREAK_IF( sk::Severity::kEditor,
        _getActiveBuilder() == eActiveBuilder::kSelf,
        "Error: You forgot to call Complete on ContextMenu." )

    SK_BREAK_RET_IF( sk::Severity::kEditor, !IsValid(), "Error: Context menu isn't properly initialized." )

    m_currently_disabled_ = false;

    for( auto& item : m_items_ )
        _drawItem( item );

    if( m_currently_disabled_ )
        ImGui::EndDisabled();
}

void cContextMenu::_drawItem( sItem& _item )
{
    auto& [ name, value ] = _item;

    sVisitor visitor{
        [&]( std::monostate& )
        {
            ImGui::Text( "%s", name.c_str() );
        },
        [&]( bool*& _bool_value )
        {
            ImGui::Checkbox( name.c_str(), _bool_value );
        },
        [&]( const callback_t& _callback )
        {
            if( ImGui::MenuItem( name.c_str() ) )
                _callback( m_user_data_ );
        },
        [&]( const sCustom& _custom )
        {
            _custom.callback( m_user_data_ );
        },
        [&]( const eSetting& _settings )
        {
            switch( _settings ) {
            case eSetting::kSetDisabled:
                ImGui::BeginDisabled();
                m_currently_disabled_ = true;
                break;
            case eSetting::kSetEnabled:
                ImGui::EndDisabled();
                m_currently_disabled_ = false;
                break;
            }
        },
        [&]( const std::unique_ptr< cContextMenu >& _submenu )
        {
            _submenu->Draw( name );
        },
        [&]( const std::unique_ptr< sBranch >& _branch )
        {
            auto& value_map = _branch->value_map;
            const auto result = _branch->selection( m_user_data_ );

            auto [ fst, lst ] = value_map.equal_range( result );
            if( fst == value_map.end() )
            {
                auto [ default_fst, default_snd] = value_map.equal_range( _branch->default_selection );
                fst = default_fst;
                lst = default_snd;
            }

            for( ; fst != lst; ++fst )
                _drawItem( fst->second );
        }
    };

    std::visit( visitor, value );
}

void cContextMenu::_add( sItem&& _value )
{
    switch( _getActiveBuilder() )
    {
    case eActiveBuilder::kChild:
    case eActiveBuilder::kNone:
    {
        SK_BREAK_IF( sk::Severity::kEditor, true, "You're not supposed to add anything at this time." )
    }
    break;
    case eActiveBuilder::kSelf:
        m_items_.emplace_back( std::move( _value ) );
    break;
    case eActiveBuilder::kIf:
    {
        auto& builder = *m_if_builders_.top();
        builder.values_map.emplace( builder.active_if_index, std::move( _value ) );
    }
    break;
    case eActiveBuilder::kSwitch:
    {
        auto& builder = *m_switch_builder_stack_.top();
        builder.value_map.emplace( builder.active_switch_case, std::move( _value ) );
    }
    break;
    default: break;
    }
}
