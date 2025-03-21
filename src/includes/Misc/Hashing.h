/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Types/types.h"

// FNV1a c++11 constexpr compile time hash functions, 32 and 64 bit
// str should be a null terminated string literal, value should be left out
// e.g. hash_32_fnv1a_const("example")
// code license: public domain or equivalent
// post: https://notes.underscorediscovery.com/constexpr-fnv1a/
// "inspiration" found at: https://gist.github.com/ruby0x1/81308642d0325fd386237cfa3b44785c

namespace Hashing
{
	constexpr uint32_t val_32_const   = 0x811c9dc5;
	constexpr uint32_t prime_32_const = 0x1000193;
	constexpr uint64_t val_64_const   = 0xcbf29ce484222325;
	constexpr uint64_t prime_64_const = 0x100000001b3;

	constexpr uint32_t fnv1a_32( const char* const _s, const uint32_t _v = val_32_const ) noexcept {
		return ( _s[ 0 ] == '\0' ) ? _v : fnv1a_32( &_s[ 1 ], ( _v ^ static_cast< uint32_t >( static_cast< uint8_t >( _s[ 0 ] ) ) ) * prime_32_const );
	}

	constexpr uint64_t fnv1a_64( const char* const _s, const uint64_t _v = val_64_const ) noexcept {
		return ( _s[ 0 ] == '\0' ) ? _v : fnv1a_64( &_s[ 1 ], ( _v ^ static_cast< uint64_t >( static_cast< uint8_t >( _s[ 0 ] ) ) ) * prime_64_const );
	}

	constexpr uint32_t fnv1a_32( const char* const _s, const size_t _c, const uint32_t _v = val_32_const ) noexcept {
		return ( _c == 0 ) ? _v : fnv1a_32( _s, _c - 1, ( _v ^ static_cast< uint32_t >( static_cast< uint8_t >( _s[ _c ] ) ) ) * prime_32_const );
	}

	constexpr uint64_t fnv1a_64( const char* const _s, const size_t _c, const uint64_t _v = val_64_const ) noexcept {
		return ( _c == 0 ) ? _v : fnv1a_64( _s, _c - 1, ( _v ^ static_cast< uint64_t >( static_cast< uint8_t >( _s[ _c ] ) ) ) * prime_64_const );
	}

	inline uint32_t fnv1a_32( const std::string& _s ) noexcept {
		return fnv1a_32( _s.c_str() );
	}

	inline uint64_t fnv1a_64( const std::string& _s ) noexcept {
		return fnv1a_64( _s.c_str() );
	}


	template< class Ty >
	constexpr uint32_t fnv1a_32( const Ty& _in, const uint32_t _v = val_32_const ) noexcept {
		return fnv1a_32( static_cast< const char* >( &_in ), sizeof( _in ), _v );
	}

	template< class Ty >
	constexpr uint64_t fnv1a_64( const Ty& _in, const uint64_t _v = val_64_const ) noexcept {
		return fnv1a_64( static_cast< const char* >( &_in ), sizeof( _in ), _v );
	}

#define HASH_REQUIREMENTS( Class ) \
constexpr Class( const Class& _other ) : iHashed( _other.m_hash ){}; \
constexpr Class( Class&& _other ) noexcept : iHashed( _other.m_hash ){} \
~Class( void ) = default; \
constexpr Class& operator=( const Class& ) = default; \
constexpr Class& operator=( Class&& _other ) noexcept { m_hash = _other.m_hash;  return *this; } \
constexpr bool   operator==( const Class& _other ) const { return m_hash == _other.m_hash; }

	class iHashed
	{
	protected:
		explicit constexpr iHashed( const uint64_t _hash )
		: m_hash( _hash ){}
	public:
		constexpr iHashed( const iHashed& _other ) = default;

		constexpr iHashed( iHashed&& _other ) noexcept
		: m_hash( _other.m_hash ){}

		~iHashed( void ) = default;

		constexpr iHashed& operator=( const iHashed& ) = default;

		constexpr iHashed& operator=( iHashed&& _other ) noexcept {
			m_hash = _other.m_hash;
			return *this;
		}

		constexpr bool operator==( const iHashed& _other ) const { return m_hash == _other.m_hash; }

		constexpr auto& getHash( void ) const { return m_hash; }

protected:
		uint64_t m_hash;
	};

} // Hashing

namespace qw
{
	template< class Ty >
		class hash : public Hashing::iHashed
	{
	public:
		constexpr hash( const Ty& _to_hash )
		: iHashed( fnv1a_64( _to_hash ) ){}
	};

	// TODO: Make name class like unreal
	template<>
	class hash< std::string > : public Hashing::iHashed
	{
	public:
		constexpr hash( const std::string& _to_hash )
		: iHashed( Hashing::fnv1a_64( _to_hash.c_str() ) ){}

		constexpr hash( const std::pmr::string& _to_hash )
		: iHashed( Hashing::fnv1a_64( _to_hash.c_str() ) ){}

		constexpr hash( const char* _to_hash )
		: iHashed( Hashing::fnv1a_64( _to_hash ) ){}

		constexpr hash( const char* _to_hash, const size_t _c )
		: iHashed( Hashing::fnv1a_64( _to_hash, _c, Hashing::val_64_const ) ){}

		HASH_REQUIREMENTS( hash )
	};

	template<>
	class hash< uint32_t > : public Hashing::iHashed
	{
	public:
		constexpr hash( const uint32_t& _to_hash )
		: iHashed( static_cast< uint64_t >( _to_hash ) ){}

		HASH_REQUIREMENTS( hash )
	};

	template<>
	class hash< uint64_t > : public Hashing::iHashed
	{
	public:
		constexpr hash( const uint64_t& _to_hash )
		: iHashed( _to_hash ){}

		HASH_REQUIREMENTS( hash )
	};
} // qw::

template< class Ty >
struct std::hash< qw::hash< Ty > >
{
	uint64_t operator()( const qw::hash< Ty >& _hash ) const
	{
		return _hash.getHash();
	}
};