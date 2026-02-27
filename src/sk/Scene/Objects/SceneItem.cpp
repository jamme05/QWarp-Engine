//
// Created by willness on 2026-02-25.
// Copyright (c) 2026 William Ask S. Ness. All rights reserved.
//

#include "SceneItem.h"

#include <sk/Scene/Scene.h>

using namespace sk::Object;

cSceneItem::cSceneItem()
: m_scene_( nullptr )
, m_uuid_( GenerateRandomUUID() )
{

}

void cSceneItem::Destroy()
{
    destroySelf();
}

void cSceneItem::Destroy( const cWeak_Ptr< cSceneItem >& _target )
{
    auto& scene = *_target->m_scene_;

    scene.m_marked_for_removal_.emplace_back( _target );
}
