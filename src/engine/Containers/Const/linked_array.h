/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <Containers/Const/array.h>

namespace qw
{
    template< typename Ty >
    class cLinked_Iterator
    {
    public:
        constexpr cLinked_Iterator( void )
        : m_next( nullptr )
        , m_element( nullptr )
        {}
        constexpr cLinked_Iterator( const Ty* _element, const cLinked_Iterator* _next )
        : m_next( _next )
        , m_element( _element )
        { }
        constexpr cLinked_Iterator( cLinked_Iterator const& _other )
        : m_next( _other.m_next )
        , m_element( _other.m_element )
        { }
        constexpr cLinked_Iterator( cLinked_Iterator && _other ) noexcept
        : m_next( _other.m_next )
        , m_element( _other.m_element )
        {
        }

        constexpr auto get_Next( void ) const { return *m_next; }

        constexpr auto& operator++( void )
        {
            if( m_next )
                *this = *m_next;
            else
                m_element = nullptr;
            return *this;
        } // ++self

        constexpr auto operator++( int )
        {
            auto tmp = *this;
            if( m_next )
                *this = *m_next;
            else
                m_element = nullptr;
            return tmp;
        } // self++

        constexpr auto operator*( void ) const
        {
            return *m_element;
        } // *self

        constexpr auto operator->( void ) const
        {
            return m_element;
        } // self->

        constexpr auto& operator=( const cLinked_Iterator& _right )
        {
            if( this == &_right )
                return *this;
            m_next    = _right.m_next;
            m_element = _right.m_element;
            return *this;
        } // self = (copy)

        constexpr auto& operator=( cLinked_Iterator&& _right ) noexcept
        {
            m_next    = _right.m_next;
            m_element = _right.m_element;
            return *this;
        } // self = (move)

        constexpr auto operator==( cLinked_Iterator& _right ) const
        {
            return m_element == _right.m_element;
        } // self == right

        constexpr auto operator!=( cLinked_Iterator& _right ) const
        {
            return m_element != _right.m_element;
        } // self != right

    private:
        cLinked_Iterator* m_next;
        Ty*               m_element;
    };

    template< typename Ty, bool HasElement = false >
    class cLinked_Array
    {
    public:
        consteval cLinked_Array( void ) = default;

        constexpr auto begin( void ) const { return nullptr; }
        constexpr auto end  ( void ) const { return nullptr; }
    };

    template< typename Ty >
    class cLinked_Array< Ty, true >
    {
    public:
        constexpr cLinked_Array( Ty&& _element);
        constexpr cLinked_Array( Ty&& _element, const cLinked_Array& _next );
        constexpr cLinked_Array( const cLinked_Array& _other );
        constexpr cLinked_Array( cLinked_Array&& _other ) noexcept;


    private:
        Ty m_element;
        cLinked_Iterator< Ty > m_iterator;
    };

    template< typename Ty >
    constexpr cLinked_Array< Ty, true >::cLinked_Array( Ty&& _element )
    : m_element( std::move( _element ) )
    , m_iterator( &m_element, nullptr )
    {
    } // sLinked_Array

    template< typename Ty >
    constexpr cLinked_Array< Ty, true >::cLinked_Array( Ty&& _element, const cLinked_Array& _next )
    : m_element( std::move( _element ) )
    , m_iterator( &m_element, _next.m_iterator )
    {
    } // sLinked_Array

    template< typename Ty >
    constexpr cLinked_Array< Ty, true >::cLinked_Array( const cLinked_Array& _other )
    : m_element( _other.m_element )
    , m_iterator( &m_element, _other.m_iterator.get_Next() )
    {
    } // cLinked_Array

    template< typename Ty >
    constexpr cLinked_Array< Ty, true >::cLinked_Array( cLinked_Array&& _other ) noexcept
    : m_element( std::move( _other.m_element ) )
    , m_iterator( std::move( _other.m_iterator ) )
    {
    } // sLinked_Array

    template< typename Ty >
    cLinked_Array( Ty ) -> cLinked_Array< Ty, true >;
    template< typename Ty >
    cLinked_Array( Ty, cLinked_Array< Ty, true > ) -> cLinked_Array< Ty, true >;
} // qw::

inline void tmp()
{
    constexpr qw::cLinked_Array arr1{ 10 };
    constexpr qw::cLinked_Array arr2{ 10, arr1 };
}
