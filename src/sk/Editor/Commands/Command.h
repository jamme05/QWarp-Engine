//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//

#pragma once

#include <string>

namespace sk::Commands
{
    class aCommand
    {
    public:
        explicit aCommand( std::string _action );
        virtual ~aCommand() = 0;

        auto& GetAction() const { return m_action_; }

        virtual void Apply() = 0;
        virtual void Undo () = 0;

    private:
        std::string m_action_;
    };
} // sk::Commands::