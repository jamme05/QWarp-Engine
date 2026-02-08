
#pragma once

#include <sk/Misc/StringID.h>

#include <cstdint>
#include <optional>

// TODO: Usage of the layers.
namespace sk::Scene
{
    // TODO: Serialization
    struct sLayer
    {
        constexpr explicit sLayer( const uint8_t _layer ) : m_layer_( _layer )
        {
            if( !Memory::is_power_of_two( _layer ) || _layer > sizeof( uint64_t ) )
            {
                SK_BREAK;
                m_layer_       = 0;
                m_layer_value_ = 0;
            }
            else
                m_layer_value_ = 1llu << _layer;
        }

        explicit sLayer( const cStringID& _layer_name );

        constexpr sLayer( const sLayer& ) = default;
        constexpr sLayer( sLayer&& ) noexcept = default;
        constexpr sLayer& operator=( const sLayer& ) = default;
        constexpr sLayer& operator=( sLayer&& ) noexcept = default;

        bool IsValid() const;

        constexpr auto GetLayer() const { return m_layer_; }
        auto GetLayerName() -> std::optional< cStringID >;
    private:
        // Works for index access as well
        uint64_t m_layer_;
        uint64_t m_layer_value_;
    };

    struct sLayerSelection
    {
        constexpr explicit sLayerSelection( const uint64_t _layers ) : m_layers_( _layers ){}

        constexpr auto GetLayer() const { return m_layers_; }

        static const sLayerSelection kAll;
    private:
        // Works for index access as well
        uint64_t m_layers_;
    };

    constexpr sLayerSelection sLayerSelection::kAll{ std::numeric_limits< uint64_t >::max() };
} // sk::Scene