/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cAsset_List.h"

namespace qw::Assets
{
	cAsset_List::~cAsset_List( void )
	{
		m_assets.clear();
	} // ~cAsset_List

	cAsset_List::cAsset_List( const cAsset_List& _other )
	{
		m_assets = _other.m_assets;
	} // cAsset_List Copy

	cAsset_List::cAsset_List( cAsset_List&& _other ) noexcept
	{
		m_assets = std::move( _other.m_assets );
	} // cAsset_List Move

	cAsset_List& cAsset_List::operator=( const cAsset_List& _other )
	{
		if( this != &_other )
			m_assets = _other.m_assets;

		return *this;
	} // operator= ( Copy )

	cAsset_List& cAsset_List::operator=( cAsset_List&& _other ) noexcept
	{
		m_assets = std::move( _other.m_assets );
		return *this;
	} // operator= ( Move )

	cAsset_List& cAsset_List::operator+=( const cAsset_List& _other )
	{
		m_assets.insert( _other.m_assets.begin(), _other.m_assets.end() );
		return *this;
	} // operator+= ( Copy )

	cAsset_List& cAsset_List::operator+=( cAsset_List&& _other )
	{
		m_assets.merge( std::move( _other ).m_assets );
		return *this;
	} // operator+= ( Move )

	void cAsset_List::add_asset( const cShared_ptr< iAsset >& _asset )
	{
		m_assets.insert( { _asset->getClassType(), _asset } );
	} // add_asset

	void cAsset_List::remove_asset( const cShared_ptr<iAsset>& _asset )
	{
		auto range = m_assets.equal_range( _asset->getClassType() );
		for( auto it = range.first; it != range.second; ++it )
		{
			if( it->second == _asset )
				m_assets.erase( it );
		}
	} // remove_asset

} // qw::Assets::
