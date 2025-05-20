/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <unordered_map>
#include <filesystem>

#include "Types/types.h"

#include "Memory/Tracker/cTracker.h"
#include "Runtime/cRuntimeClass.h"
#include "Misc/Hashing.h"
#include "Misc/Smart_ptrs.h"

namespace qw
{
	class cAssetManager;

	// Remember to update macro if changing this.
	typedef uint32_t hash_t;

	// Base Asset interface
	GENERATE_ALL_CLASS( iAsset )
	public:
		explicit iAsset( std::string _name )
		: m_name( std::move( _name ) )
		, m_name_hash( m_name )
		{ }
		virtual ~iAsset( void ) = default;

		auto&     getPath ( void ) const { return m_path; }

		auto getNameHash( void ) const { return m_name_hash; }

		virtual void Save( void ) = 0;

	private:
		Asset_id_t  m_id   = INVALID_ASSET_ID;

		void set_path( const std::filesystem::path& _path ){ m_path = _path; m_path_hash = m_path; }

	protected:
		std::string           m_name;
		str_hash              m_name_hash;
		std::filesystem::path m_path      = {};
		str_hash              m_path_hash = string_hash_none;

	friend class cAssetManager;

	};

	// Base Asset class
	template< class Ty, class RTClassTy, const RTClassTy& RTClass >
	requires std::is_base_of_v< iRuntimeClass, RTClassTy >
	class cAsset : public iAsset
	{
		CREATE_CLASS_IDENTIFIERS( RTClass )
	public:

		cAsset( std::string _name ) : iAsset( std::move( _name ) ){ }

		template< class... Args >
		static Ty* create( Args... _args ){ return QW_NEW( Ty, 1, _args... ); }

		template< class... Args >
		static cShared_ptr< Ty > create_shared( Args... _args ){ return qw::cShared_ptr< Ty >( QW_NEW( Ty, 1, _args... ) ); }

		void Save( void ) override { /* Saving not supported */ }

	friend Ty;
	};

	typedef iAsset Asset_t;

	template< class In, class Out >
	using enable_if_asset_t = std::enable_if_t< std::is_base_of_v< iAsset, In >, Out >;
	template< class Ty >
	static constexpr bool is_valid_asset_v = std::is_base_of_v< iAsset, Ty >;

} // qw::

// Initializes data required for cAsset.
// The class created by this will be using the default naming (ex: Mesh becomes cMesh).
// Raw values can be accessed in qw::Assets::[Name], (ex qw::Assets::Mesh)
#define ASSET_PARENT_CLASS( AssetName, ... ) qw::iAsset
#define ASSET_PARENT_VALIDATOR( AssetName, ... ) qw::is_valid_asset_v< __VA_ARGS__ >
#define ASSET_PARENT_CREATOR_2( AssetName, ... ) AFTER_FIRST( __VA_ARGS__ )
#define ASSET_PARENT_CREATOR_1( AssetName, ... ) qw::cAsset< M_CLASS( AssetName ), AssetName::runtime_class_t, AssetName :: CONCAT( runtime_class_, AssetName ) >
#define ASSET_PARENT_CREATOR( AssetName, ... ) CONCAT( ASSET_PARENT_CREATOR_, VARGS( __VA_ARGS__ ) ) ( AssetName, __VA_ARGS__ )
#define QW_ASSET_CLASS( AssetName, ... ) QW_RESTRICTED_CLASS( AssetName, ASSET_PARENT_CLASS, ASSET_PARENT_CREATOR, ASSET_PARENT_VALIDATOR, EMPTY __VA_OPT__( , __VA_ARGS__ ) )
