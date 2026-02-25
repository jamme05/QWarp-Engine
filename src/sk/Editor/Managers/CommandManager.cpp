//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//


#include "CommandManager.h"

using namespace sk::Managers;

cCommandManager::cCommandManager( size_t _history_size )
{
    m_history_.resize( _history_size );
}

bool cCommandManager::Redo()
{
    // Redo the current position
    if( m_position_ == m_last_ )
        return false;


}

bool cCommandManager::Undo()
{
    if( m_position_ == m_start_ )
        return false;


}

void cCommandManager::Submit( std::unique_ptr< Commands::aCommand >&& _command )
{
    _command->Apply();
    m_history_[ m_position_ ] = std::move( _command );

    if( m_position_ == m_start_ )
        _inc( m_start_ );

    if( _inc( m_position_ ) != m_last_ )
    {
        _clearAfterPosition();
        // We're not at the latest change, so we're gonna need to discard some history.

    }
    m_last_ = m_position_;
}

void cCommandManager::SetHistorySize( size_t _new_history_size )
{
    command_vec_t new_history( _new_history_size );

    if( _new_history_size > _size() )
    {
        // We can safely copy everything
        for( size_t i = 0, p = m_start_; p != m_position_; _inc( p ), i++ )
            new_history[ i ].swap( m_history_[ p ] );
    }
    else
    {
        const auto diff = _size() - _new_history_size;
        size_t start;
        if( m_start_ > m_position_ )
            start = ( m_start_ + diff ) % _new_history_size;
        else
            start = m_start_ + diff;

        for( size_t i = 0; i < _new_history_size; i++, start++ )
            new_history[ i ].swap( m_history_[ start ] );
    }
    m_history_ = std::move( new_history );
}

auto cCommandManager::_inc( size_t& _v ) const -> size_t
{
    const size_t tmp = _v;
    _v = ( _v + 1 ) % m_history_.size();
    return tmp;
}

auto cCommandManager::_size() const -> size_t
{
    // Position has wrapped around
    if( m_start_ > m_position_ )
        return m_position_ + ( m_history_.size() - ( m_history_.size() - m_start_ ) );
    return m_position_ - m_start_;
}

void cCommandManager::_clearAfterPosition()
{
    for( size_t i = m_position_; i != m_last_; _inc( i ) )
        m_history_[ i ].reset();

    m_last_ = m_position_ - 1;
}
