
#pragma once

#include <sk/Misc/StringID.h>

#include <cstdint>
#include <optional>

// TODO: Usage of the layers.
namespace sk::Physics
{
    // TODO: Serialization
    struct sPhysicsLayer
    {
        constexpr explicit sPhysicsLayer( const uint8_t _layer ) : m_layer_( _layer )
        {
            if( !Memory::is_power_of_two( _layer ) || _layer > sizeof( uint32_t ) )
            {
                SK_BREAK;
                m_layer_       = 0;
                m_layer_value_ = 0;
            }
            else
                m_layer_value_ = 1lu << _layer;
        }

        explicit sPhysicsLayer( const cStringID& _layer_name );

        constexpr sPhysicsLayer( const sPhysicsLayer& ) = default;
        constexpr sPhysicsLayer( sPhysicsLayer&& ) noexcept = default;
        constexpr sPhysicsLayer& operator=( const sPhysicsLayer& ) = default;
        constexpr sPhysicsLayer& operator=( sPhysicsLayer&& ) noexcept = default;

        bool IsValid() const;

        constexpr auto GetLayer() const { return m_layer_; }
        auto GetLayerName() -> std::optional< cStringID >;
    private:
        // Works for index access as well
        uint32_t m_layer_;
        uint32_t m_layer_value_;
    };

    struct sPhysicsLayerSelection
    {
        constexpr explicit sPhysicsLayerSelection( const uint64_t _layers ) : m_layers_( _layers ){}

        constexpr auto GetLayer() const { return m_layers_; }

        static const sPhysicsLayerSelection kAll;
    private:
        // Works for index access as well
        uint32_t m_layers_;
    };

    constexpr sPhysicsLayerSelection sPhysicsLayerSelection::kAll{ std::numeric_limits< uint64_t >::max() };
} // sk::Scene