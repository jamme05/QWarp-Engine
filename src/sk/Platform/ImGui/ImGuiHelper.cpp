//
// Created by willness on 2026-02-27.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//

#include "ImGuiHelper.h"

#include <imgui.h>

using namespace sk::Gui;

bool sk::Gui::BeginCentered( const char* _name )
{
    const auto& io = ImGui::GetIO();
    ImVec2 pos{ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f };
    ImGui::SetNextWindowPos( pos, ImGuiCond_Always, ImVec2( 0.5f, 0.5f ) );
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings;

    return ImGui::Begin( _name, nullptr, flags );
}