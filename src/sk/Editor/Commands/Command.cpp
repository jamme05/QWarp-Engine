//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#include "Command.h"

using namespace sk::Commands;

aCommand::aCommand( std::string _action )
: m_action_( std::move( _action ) )
{}
