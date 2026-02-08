
#pragma once

#include "Layer.h"

using namespace sk::Scene;

sLayer::sLayer( const cStringID& _layer_name )
{

}

bool sLayer::IsValid() const
{
    return m_layer_value_ != 0;
}

auto sLayer::GetLayerName() -> std::optional< cStringID >
{

}
