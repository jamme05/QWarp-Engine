/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Containers/Const/array.h"

// TODO: Make a template specialization for array< char, Size >.

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

    template< class Ty, class Ty2 = Ty >
    concept is_valid_string = std::is_same_v< Ty, char > && std::is_same_v< Ty, Ty2 >;

    template< int64_t Start, int64_t End, array Str >
    requires is_valid_string< arr::type< Str > >
    struct substr_internal
    {
        constexpr static size_t offset = get_range( Str.size(), Start, End ).first;
        constexpr static size_t size   = get_range( Str.size(), Start, End ).second;
        constexpr substr_internal( void )
        {
            std::copy_n( Str.value + offset, size, value );
        } // substr_calc
        char value[ size + 1 ]{};
    };

    template< int64_t Start, array Str, int64_t End = -1 >
    requires is_valid_string< arr::type< Str > >
    struct substr
    {
        constexpr static substr_internal< Start, End, Str > compute{};
        constexpr static const char*   kValue = compute.value;
        constexpr static array kForward{ compute.value };
    };

    template< size_t StrSize, size_t SearchSize >
    constexpr static int64_t find_index_of( const char ( &_str )[ StrSize ], const char ( &_search )[ SearchSize ], const bool _backwards = false )
    {
        // TODO: Write better.
        static_assert( StrSize >= SearchSize, "Error: Target string has to be larger than the one to find." );
        // Remove null ending from check.
        const auto str_size    = StrSize - ( _backwards ? 2 : 1 );
        constexpr auto search_size = SearchSize - 2;

        int64_t found   = -1;
        size_t  counter = 0;

        if( _backwards )
        {
            size_t index = search_size;
            for( int64_t i = static_cast< int64_t >( str_size ); i >= 0; --i )
            {
                if( _str[ i ] == _search[ index ] )
                {
                    --index;
                    found = i;
                    if( counter++ == search_size )
                        return found; // Found
                }
                else
                {
                    if( found != -1 )
                    {
                        i = found;
                        found   = -1;
                    }
                    counter = 0;
                    index   = search_size;
                }
            }
        }
        else
        {
            for( size_t i = 0; i < str_size; ++i )
            {
                if( _str[ i ] == _search[ counter ] )
                {
                    if( found == -1 )
                        found = static_cast< int64_t >( i );
                    if( counter++ == search_size )
                        return found; // Found
                }
                else
                {
                    if( found != -1 )
                    {
                        i = static_cast< size_t >( found );
                        found = -1;
                    }
                    counter = 0;
                }
            }
        }

        return -1;
    } // find_index_of

    template< array Str, array Find, bool Backwards = false >
    requires is_valid_string< arr::type< Str >, arr::type< Find > >
    struct find
    {
        constexpr static int64_t kIndex = find_index_of( Str.value, Find.value, Backwards );
        constexpr static bool    kFound = kIndex != -1;
    };

    template< array Str, array Other >
    requires is_valid_string< arr::type< Str >, arr::type< Other > >
    struct equals
    {
        constexpr static bool kValue = Str.Length == Other.Length && find_index_of( Str.value, Other.value ) == 0;
    };

    template< array Str, array Find >
    requires is_valid_string< arr::type< Str >, arr::type< Find > >
    struct starts_with
    {
        constexpr static bool kValue = find_index_of( Str.value, Find.value ) == 0;
    };

    template< array Str, array Find >
    requires is_valid_string< arr::type< Str >, arr::type< Find > >
    struct ends_with
    {
        constexpr static bool kValue = find_index_of( Str.value, Find.value, true ) == Str.size() - Find.size();
    };
} // qw::str::