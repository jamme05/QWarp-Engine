/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <cstdint>

namespace sk::Severity
{
    enum : uint32_t
    {
        kValueMask =  static_cast< uint32_t >( std::numeric_limits< uint16_t >::max() ),
        kTypeMask  = ~kValueMask,

        // A way to use general in case it isn't specified.
        kAny        = 0,
        kGeneral    = 1u << 16,
        kEngine     = 1u << 17,
        kGraphics   = 1u << 18,
        kPlatform   = 1u << 19,
        kEditor     = 1u << 20,
        kMemory     = 1u << 21,
        kReflection = 1u << 22,

        // TODO: Maybe do the inverse?
        kConst         = 1u << 31,
        kConstGeneral  = kConst | kGeneral,
        kConstEngine   = kConst | kEngine,
        kConstGraphics = kConst | kGraphics,
        kConstPlatform = kConst | kPlatform,
        kConstEditor   = kConst | kEditor,
        kConstMemory   = kConst | kMemory,

        kAllValues = kValueMask,
        kAllTypes  = kTypeMask,
        kAll = std::numeric_limits< uint32_t >::max(),
    };
    static constexpr auto GetValue( const uint32_t _severity ) noexcept { return kValueMask & _severity; }
    static constexpr auto GetType ( const uint32_t _severity ) noexcept { return kTypeMask & _severity; }

    // TODO: Add function to math as nearest power of two.
    // https://stackoverflow.com/a/466242

    // Creates a mask allowing any value between 0 and _value. NOTE that it will a few above the value.
    static constexpr uint32_t Range( uint_fast16_t _value )
    {
        for( int_fast8_t s = 16; s >= 0; --s )
            _value |= _value >> s;
                
        return static_cast< uint32_t >( _value );
    }

    struct sSeverity_Filter
    {
        constexpr sSeverity_Filter( const uint32_t _filter ) noexcept
        : m_filter( _filter )
        , m_value ( GetValue( _filter ) )
        , m_type  ( GetType ( _filter ) )
        {} // sSeverity_Filter

        uint32_t m_filter;
        uint32_t m_value;
        uint32_t m_type;
    };

    static constexpr sSeverity_Filter kConstFilter = 0;
    static constinit sSeverity_Filter kFilter      = kConstFilter;

    static constexpr bool IsConst( const uint32_t _severity ) noexcept { return _severity & kConst; }
    static constexpr bool IsEnabled( const uint32_t _severity ) noexcept
    {
        uint32_t filter_value;
        uint32_t filter_type;

        if consteval
        {
            filter_value = kConstFilter.m_value;
            filter_type  = kConstFilter.m_type;
        }
        else
        {
            filter_value = kFilter.m_value;
            filter_type  = kFilter.m_type;
        }

        const auto value = GetValue( _severity ) | filter_value;
        auto       type  = GetType ( _severity );
        if( type == kAny )
            type = kGeneral;

        return ( value <= filter_value ) && ( type & filter_type );
    }
} // sk::Severity::

// TODO: Add runtime variant.
#define SK_PASSTHROUGH_0_( SeverityValue, ... ){ \
    static constexpr bool kSeverityVal = sk::Severity::IsConst( SeverityValue ) && sk::Severity::IsEnabled( SeverityValue ); \
    if( kSeverityVal || sk::Severity::IsEnabled( SeverityValue ) ){ __VA_ARGS__ } }

#define SK_PASSTHROUGH( SeverityValue, ... ) \
    SK_PASSTHROUGH_0_( SeverityValue, __VA_ARGS__ )
