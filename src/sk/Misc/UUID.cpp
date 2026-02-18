/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "UUID.h"

#include <sk/Seralization/SerializedObject.h>

#include <random>

namespace sk
{
    namespace
    {
        // https://en.cppreference.com/w/cpp/utility/format/spec.html
        // Is there any way to clean up the 16 uint8_t?...
        using uuid_format_string_t  = std::format_string< uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t >;
        constexpr uuid_format_string_t dashed_format_string { "{:0>2x}{:0>2x}{:0>2x}{:0>2x}-{:0>2x}{:0>2x}-{:0>2x}{:0>2x}-{:0>2x}{:0>2x}-{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}" };
        constexpr uuid_format_string_t compact_format_string{ "{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}{:0>2x}" };
    } // ::

    cUUID::cUUID( cSerializedObject& _object )
    {
        
    }

    auto cUUID::Serialize() const -> cSerializedObject
    {
        cSerializedObject object{};
        object.BeginWrite();
        object.WriteValue( "_value", ToString() );
        object.EndWrite();
        return object;
    }

    cUUID cUUID::FromString( const std::string_view& _raw_uuid )
    {
        SK_BREAK_RET_IF( sk::Severity::kEngine, _raw_uuid.size() < 32, "UUID size is less than minimum of 32 characters.", kInvalid )

        cUUID uuid;

        size_t     uuid_length = 0;
        const auto raw_uuid = reinterpret_cast< uint8_t* >( &uuid );
        for( auto c : _raw_uuid )
        {
            if( c == '-' )
                continue;

            c = Math::CharToHex( c );

            SK_BREAK_RET_IF( sk::Severity::kEngine, c == -1, "Warning: Invalid UUID string provided.", kInvalid )
            const auto index = uuid_length / 2;

            if( uuid_length % 2 )
                raw_uuid[ index ] = c;
            else
                raw_uuid[ index ] = c << 4;

            ++uuid_length;
        }

        return uuid;
    }

    std::string cUUID::ToString( const bool _dashed ) const
    {
        std::string_view target_string;

        if( _dashed )
            target_string = dashed_format_string.get();
        else
            target_string = compact_format_string.get();

        // I hate this, but I hope it works...
        const auto b = reinterpret_cast< const uint8_t* >( &m_low_ );

        return std::vformat(
            target_string,
            std::make_format_args(
                b[ 0 ], b[ 1 ], b[  2 ], b[  3 ], b[  4 ], b[  5 ], b[  6 ], b[  7 ],
                b[ 8 ], b[ 9 ], b[ 10 ], b[ 11 ], b[ 12 ], b[ 13 ], b[ 14 ], b[ 15 ]
            )
        );
    } // to_string
    
    namespace
    {
        // This is following the RFC 9562 standard for the UUIDv4 ( https://www.rfc-editor.org/rfc/rfc9562.html ). 
        // We're working in a Byte environment hence we need to convert it.
        constexpr uint8_t uuid_version_index = 48 / 8;
        constexpr uint8_t uuid_variant_index = 64 / 8;
        constexpr uint8_t uuid_version_mask  = 0b00001111;
        constexpr uint8_t uuid_variant_mask  = 0b00111111;

        constexpr uint8_t uuid_v4_version = 0b01000000;
        constexpr uint8_t uuid_v4_variant = 0b10000000;
    } // ::
    
    cUUID GenerateRandomUUID()
    {
        static std::random_device rd;
        static std::mt19937_64    gen{ rd() };
        
        const auto low  = gen();
        const auto high = gen();

        cUUID uuid{ low, high };
        const auto raw = reinterpret_cast< uint8_t* >( &uuid );

        // Add the version and variant.
        raw[ uuid_version_index ] &= uuid_version_mask;
        raw[ uuid_version_index ] |= uuid_v4_version;

        raw[ uuid_variant_index ] &= uuid_variant_mask;
        raw[ uuid_variant_index ] |= uuid_v4_variant;

        return uuid;
    } // GenerateRandomUUID
} // sk::
