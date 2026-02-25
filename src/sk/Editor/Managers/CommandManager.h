//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#pragma once

#include <sk/Editor/Commands/Command.h>
#include <sk/Misc/Singleton.h>

#include <memory>
#include <vector>

namespace sk::Managers
{
    class cCommandManager : public cSingleton< cCommandManager >
    {
    public:
        explicit cCommandManager( size_t _history_size = 50 );

        bool Redo();
        bool Undo();

        void Submit( std::unique_ptr< Commands::aCommand >&& _command );

        void SetHistorySize( size_t _new_history_size );

    private:
        // value++ which stays within the range of the ringbuffer.
        auto _inc( size_t& _v ) const -> size_t;
        auto _size() const -> size_t;
        void _clearAfterPosition();
        using command_vec_t = std::vector< std::unique_ptr< Commands::aCommand > >;

        // These will have the vector be treated as a ringbuffer.
        size_t        m_start_    = 0;
        // Position will always be the next element we'll be writing to.
        size_t        m_position_ = 0;
        size_t        m_last_      = 0;
        command_vec_t m_history_;
    };
} // sk::Managers::

namespace sk::Commands
{
    template< class Ty, class... Args >
    requires std::constructible_from< Ty, Args... >
    void Submit( Args&&... _args )
    {
        Managers::cCommandManager::get().Submit( std::make_unique< Ty >( std::forward< Args >( _args )... ) );
    }

    bool Redo( const size_t _nr_of_commands )
    {

    }

    bool Undo( const size_t _nr_of_commands )
    {

    }
} // sk::Commands::