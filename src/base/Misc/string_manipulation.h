/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

namespace qw::str
{
    constexpr static int64_t get_single_size( const size_t _size, const int64_t _value )
    {
        if( _value == -1 )
            return static_cast< int64_t >( _size ) - 1;
        if( _value < 0 )
            return static_cast< int64_t >( _size ) + _value - 1;

        return _value;
    }
    
    constexpr static std::pair< size_t, size_t > get_range( const size_t _length, const int64_t _start, const int64_t _end )
    {
        const auto start = get_single_size( _length, _start );
        const auto end   = get_single_size( _length, _end );

        if( start > end || start < 0 || end < 0 )
            return { 0, 0 };
        
        return { start, static_cast< size_t >( end - start ) };
    } // get_index

    template< size_t N >
    struct StringLiteral
    {
        constexpr StringLiteral( const char ( &str )[ N ])
        {
            std::copy_n(str, N, value);
        }
        constexpr static auto Length = N;

        char value[ N ];
    };

    template< int64_t Start, int64_t End, StringLiteral Str >
    struct substr_internal
    {
        constexpr static size_t offset = get_range( Str.Length, Start, End ).first;
        constexpr static size_t size   = get_range( Str.Length, Start, End ).second;
        constexpr substr_internal( void )
        {
            std::copy_n( Str.value + offset, size, value );
        } // substr_calc
        char value[ size + 1 ]{};
    };

    template< int64_t Start, StringLiteral Str, int64_t End = -1 >
    struct substr
    {
        constexpr static substr_internal< Start, End, Str > compute{};
        constexpr static const char*   kValue = compute.value;
        constexpr static StringLiteral kForward{ compute.value };
    };

    template< size_t StrSize, size_t SearchSize >
    constexpr static int64_t find_index_of( const char ( &_str )[ StrSize ], const char ( &_search )[ SearchSize ], const bool _backwards = false )
    {
        // TODO: Write better.
        static_assert( StrSize >= SearchSize, "Error: Target string has to be larger than the one to find." );
        // Remove null ending from check.
        const auto str_size    = StrSize - ( _backwards ? 2 : 1 );
        constexpr auto search_size = SearchSize - 2;

        const size_t end = _backwards ? 0 : str_size;

        int64_t found   = -1;
        size_t  counter = 0;
        size_t  index   = _backwards ? search_size : 0;
        size_t i = _backwards ? str_size : 0;
        while( i != end )
        {
            if( _str[ i ] == _search[ index ] )
            {
                _backwards ? index-- : index++;

                if( _backwards || found == -1 )
                    found = static_cast< int64_t >( i );
                if( counter++ == search_size )
                    return found; // Found
            }
            else
            {
                if( _backwards && found != -1 )
                {
                    i = static_cast< size_t >( found );
                }
                else
                    i -= index;
                found   = -1;
                counter = 0;
                index   = _backwards ? search_size : 0;
            }
            if( _backwards )
                --i;
            else
                ++i;
        }
        
        return -1;
    } // find_index_of

    template< StringLiteral Str, StringLiteral Find, bool Backwards = false >
    struct find
    {
        constexpr static auto kIndex = find_index_of( Str.value, Find.value, Backwards );
    };

    template< StringLiteral Str, StringLiteral Other >
    struct equals
    {
        constexpr static bool kValue = Str.Length == Other.Length && find_index_of( Str.value, Other.value ) == 0;
    };

    template< StringLiteral Str, StringLiteral Find >
    struct starts_with
    {
        constexpr static bool kValue = find_index_of( Str.value, Find.value ) == 0;
    };

    template< StringLiteral Str, StringLiteral Find >
    struct ends_with
    {
        constexpr static bool kValue = find_index_of( Str.value, Find.value, true ) == Str.Length - Find.Length;
    };

    
} // qw::str::
