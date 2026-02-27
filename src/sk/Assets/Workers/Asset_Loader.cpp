/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Asset_Loader.h"

#include <sk/Assets/Management/Asset_Job_Manager.h>

namespace
{
    sk::Assets::Jobs::cAsset_Job_Manager* manager = nullptr;
} // ::

sk::Assets::Jobs::cAsset_Worker::cAsset_Worker()
{
    if( manager == nullptr )
        manager = cAsset_Job_Manager::getPtr();
}

void sk::Assets::Jobs::cAsset_Worker::worker( cAsset_Worker* _loader )
{
    auto& self = *_loader;

    while( self.m_active_ )
    {
        if( auto task = manager->WaitForTask( self.m_active_ ); task.type != eJobType::kNone )
        {
            self.m_working_.store( true );
            do_work( task );
            self.m_working_.store( false );
        }
    }
}

void sk::Assets::Jobs::cAsset_Worker::do_work( const sTask& _work )
{
    switch( _work.type ) {
    case eJobType::kNone: return;
    case eJobType::kLoad:
        load_asset( *static_cast< sAssetTask* >( _work.data ), false );
        break;
    case eJobType::kUnload:
        unload_asset( *static_cast< sAssetTask* >( _work.data ) );
        break;
    case eJobType::kRefresh:
        load_asset( *static_cast< sAssetTask* >( _work.data ), true );
        break;
    case eJobType::kSave:
        save_asset( *static_cast< sAssetTask* >( _work.data ) );
        break;
    case eJobType::kPushEvent:
        push_event( *static_cast< sListenerTask* >( _work.data ) );
        break;
    }

    Memory::free_fast( _work.data );
}

void sk::Assets::Jobs::cAsset_Worker::load_asset( sAssetTask& _task, const bool _refresh )
{
    const auto loader_task = _refresh ? eAssetTask::kRefreshAsset : eAssetTask::kLoadAsset;
    _task.loader( _task.path, _task.affected_assets, loader_task );
    
    for( auto& meta : _task.affected_assets )
        meta->m_dispatcher_.push_event( *meta, _refresh ? eEventType::kUpdated : eEventType::kLoaded );
}

void sk::Assets::Jobs::cAsset_Worker::save_asset( sAssetTask& _task )
{
    _task.loader( _task.path, _task.affected_assets, eAssetTask::kSaveAsset );
}

void sk::Assets::Jobs::cAsset_Worker::unload_asset( sAssetTask& _task )
{
    if( !manager->IsShuttingDown() )
    {
        for( auto& meta : _task.affected_assets )
        {
            meta->m_dispatcher_.push_event( *meta, eEventType::kUnload );
            meta->m_flags_ &= ~cAsset_Meta::kLoaded;
            meta->m_flags_ |= cAsset_Meta::kUnloading;
        }
    }
    
    _task.loader( _task.path, _task.affected_assets, eAssetTask::kUnloadAsset );

    for( auto& asset : _task.affected_assets )
        asset->setAsset( nullptr );
}

void sk::Assets::Jobs::cAsset_Worker::push_event( sListenerTask& _task )
{
    _task.event( *_task.meta, eEventType::kLoaded );
}

