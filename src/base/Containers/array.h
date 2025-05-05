/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <algorithm>
#include <utility>

namespace qw
{
    template< class Ty >
    class array_ref;

    struct array_base
    {
        constexpr array_base( void ) = default;
        constexpr array_base( const array_base& ) = default;
        constexpr array_base( array_base&& ) = default;
        constexpr array_base& operator=( const array_base& ) = default;
        constexpr array_base& operator=( array_base&& ) = default;
        virtual constexpr ~array_base( void ) = default;
    protected:
        virtual constexpr const void* _get  ( void ) const = 0;
        virtual constexpr       void* _get  ( void )       = 0;
    public:
        virtual constexpr size_t size( void ) const = 0;

        template< class Ty >
        friend class array_ref;
    };

    template< class Ty, size_t Size >
    struct array
    {
        typedef Ty value_type;
        typedef Ty ( & arr_type )[ Size ];
        constexpr static auto array_size = Size;

        constexpr array( const Ty ( &_arr )[ Size ] )
        {
            std::copy_n( _arr, Size, value );
        } // array

        template< class... Args >
        requires ( std::conjunction_v< std::is_same< Ty, Args >... > && !std::is_array_v< Ty > )
        constexpr array( Args... _values )
        : value{ std::move( _values )... }
        {
        } // array

        template< size_t Size2 >
        constexpr array< Ty, Size + Size2 > operator+( const array< Ty, Size2 >& _other ) const
        {
            if constexpr( Size2 == 0 )
            {
                return *this;
            }
            else
            {
                array< Ty, Size + Size2 > result;
                std::copy_n( value, Size, result.value );
                std::copy_n( _other.value, Size2, result.value + Size );
                return result;
            }
        }

        constexpr const Ty* begin( void ) const { return get(); }
        constexpr       Ty* begin( void )       { return get(); }
        constexpr const Ty* end  ( void ) const { return get() + Size; }
        constexpr       Ty* end  ( void )       { return get() + Size; }

        constexpr const Ty* get  ( void ) const { return value; }
        constexpr       Ty* get  ( void )       { return value; }

        constexpr const Ty& operator[]( const size_t _index ) const { return value[ _index ]; }
        constexpr       Ty& operator[]( const size_t _index )       { return value[ _index ]; }

        constexpr size_t size( void ) const { return array_size; }

        Ty value[ Size ];
    };

    template< class Ty >
    struct array< Ty, 0 > : array_base
    {
        typedef Ty value_type;
        constexpr static auto array_size = 0;

        constexpr array( void ) = default; // array

        constexpr auto begin( void ) const { return get(); }
        constexpr auto begin( void )       { return get(); }
        constexpr auto end  ( void ) const { return get(); }
        constexpr auto end  ( void )       { return get(); }

        constexpr auto get  ( void ) const { return nullptr; }

        template< size_t Size2 >
        constexpr array< Ty, Size2 > operator+( const array< Ty, Size2 >& _other ) const
        {
            array< Ty, Size2 > result;
            std::copy_n( _other.value, Size2, result.value );
            return result;
        }

        constexpr const void* _get( void ) const override { return nullptr; }
        constexpr       void* _get( void )       override { return nullptr; }

        constexpr size_t size( void ) const override { return array_size; }
    };

    template< class Ty >
    class array_ref
    {
    public:
        /**
         *
         * @attention Requires _array to be static.
         * 
         * @tparam Size The size of the array to reference.
         * @param _array The static array to reference.
         */
        template< size_t Size >
        constexpr array_ref( const array< Ty, Size >& _array );

        constexpr const Ty* begin( void ) const { return get(); }
        constexpr const Ty* end  ( void ) const { return get() + size(); }

        constexpr const Ty* get  ( void ) const { return static_cast< const Ty* >( m_data ); }

        constexpr const Ty& operator[]( const size_t _index ) const { return get()[ _index ]; }

        constexpr size_t size( void ) const { return m_size; }

    private:
        size_t    m_size;
        const Ty* m_data;
    };

    template< class Ty >
    template< size_t Size >
    constexpr array_ref< Ty >::array_ref( const array< Ty, Size >& _array )
    : m_size( _array.size() )
    , m_data( _array.get() )
    {
    } // array_ref

    namespace arr
    {
        template< array First, array Second >
        requires std::is_same_v< typename decltype( First )::value_type, typename decltype( Second )::value_type >
        struct concat
        {
            constexpr static auto kValue = First + Second;
            constexpr static auto kSize  = First.size() + Second.size();
        };
        template< array Array >
        using type = typename decltype( Array )::value_type;
    } // arr::
    template< class Ty, class... Ty2 >
    requires std::conjunction_v< std::is_same< Ty, Ty2 >... >
    array( Ty, Ty2... ) -> array< Ty, 1 + sizeof...( Ty2 ) >;
    template< class Ty, size_t Size >
    array( const Ty ( & )[ Size ] ) -> array< Ty, Size >;

    template< size_t Size >
    using string = array< char, Size >;
} // qw::