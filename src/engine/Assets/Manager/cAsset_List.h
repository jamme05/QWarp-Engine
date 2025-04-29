/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Assets/cTexture.h"
#include "Assets/iAsset.h"
#include "Misc/Smart_ptrs.h"

#include "fastgltf/core.hpp"
#include "Types/vertex.h"

#include "map"

namespace qw
{
	class cAssetManager;
} // qw::

namespace qw::Assets
{
	// A container for the newly imported assets.
	class cAsset_List
	{
		typedef std::multimap< hash< iRuntimeClass >, cShared_ptr< iAsset > > asset_map_t;
		typedef std::map< hash< iRuntimeClass >, size_t >                     asset_counter_map_t;

		class asset_iterator
		{
		public:
			asset_iterator( const asset_map_t::const_iterator& _it ) : m_it( _it ){}

			const cShared_ptr< iAsset >& operator*() const { return m_it->second; }
			operator cShared_ptr< iAsset >        () const { return m_it->second; }

			bool operator!=( const asset_iterator& _other ) const { return m_it != _other.m_it; }
			bool operator==( const asset_iterator& _other ) const { return m_it == _other.m_it; }

			asset_iterator& operator++(){ ++m_it; return *this; }
			asset_iterator& operator--(){ ++m_it; return *this; }
		private:
			asset_map_t::const_iterator m_it;
		};

	public:
		 cAsset_List( void ) = default;
		~cAsset_List( void );

		cAsset_List( const cAsset_List& _other );
		cAsset_List( cAsset_List&& _other ) noexcept;

		cAsset_List& operator=( const cAsset_List& _other );
		cAsset_List& operator=( cAsset_List&& _other ) noexcept;

		cAsset_List& operator+=( const cAsset_List& _other );
		cAsset_List& operator+=( cAsset_List&& _other );

		template< class Ty >
		requires std::is_base_of_v< iAsset, Ty >
		cShared_ptr< Ty > Get_Asset_Of_Type( void )
		{
			auto  range   = m_assets.equal_range( Ty::getStaticClassType() );

			if( range.first == m_assets.end() )
				return nullptr;

			// Always return same if only a single exists.
			if( range.first == range.second )
				return range.first->second;

			auto& counter = m_counters[ Ty::getStaticClassType() ];
			auto  dist    = std::distance( range.first, range.second );

			if( counter >= dist )
			{
				printf( "Warning: No more assets of type %s. Asset List will now loop around.", Ty::getStaticClass().getRawName() );
				counter = 0;
			}
			auto itr = std::next( range.first, counter++ );

			return itr->second;
		} // Get_Asset_Of_Type

		template< class Ty >
		enable_if_asset_t< Ty, std::vector< cShared_ptr< Ty > > >
		Get_Assets_Of_Type( const int32_t _max_count = 0 )
		{
			auto range = m_assets.equal_range( Ty::getStaticClassType() );
			std::vector< cShared_ptr< Ty > > assets;

			if( _max_count > 0 )
			{
				for( auto itr = range.first; itr != range.second; ++itr )
					assets.push_back( itr->second );
			}
			else
			{
				size_t count = 0;
				for( auto itr = range.first; itr != range.second && count < _max_count; ++itr, ++count )
					assets.push_back( itr->second );
			}

			return assets;
		} // Get_Assets_Of_Type

		auto begin( void ) const { return asset_iterator( m_assets.begin() ); }
		auto end  ( void ) const { return asset_iterator( m_assets.end  () ); }

		bool empty( void ) const { return m_assets.empty(); }

	protected:

		void add_asset   ( const cShared_ptr< iAsset >& _asset );
		void remove_asset( const cShared_ptr< iAsset >& _asset );

	private:
		friend class qw::cAssetManager;

		asset_map_t         m_assets;
		asset_counter_map_t m_counters;
	};

} // qw::Assets::
