
#pragma once

#include "PhysicsLayer.h"

using namespace sk::Physics;

sPhysicsLayer::sPhysicsLayer( const cStringID& _layer_name )
{

}

bool sPhysicsLayer::IsValid() const
{
    return m_layer_value_ != 0;
}

auto sPhysicsLayer::GetLayerName() -> std::optional< cStringID >
{

}
