#pragma once

#include <unordered_map>

#include "Assets/iAsset.h"
#include "Misc/cSingleton.h"
#include "Misc/Smart_ptrs.h"
#include "Types/types.h"

#include "fastgltf/core.hpp"
#include "Graphics/cDynamic_buffer.h"
#include "Types/vertex.h"

namespace qw
{
	class cAssetManager : public cSingleton< cAssetManager >
	{
	public:
		 cAssetManager( void );
		~cAssetManager( void );

		template< class In, class Out >
		using enable_if_asset_t = std::enable_if_t< std::is_base_of_v< iAsset, In >, Out >;

		auto getAsset( const Asset_id_t _id ) -> cShared_ptr< Asset_t >;

		template< class Ty >
		auto getAssetAs( const Asset_id_t _id ) -> cShared_ptr< Ty >
		{
			if( auto asset = getAsset( _id ); asset && asset->getType() == Ty::getAssetType() )
				return asset.cast< Ty >();

			return nullptr;
		} // getAsset

		auto getAssetByName ( const hash< std::string >& _name_hash ) -> cShared_ptr< Asset_t >;
		auto getAssetsByName( const hash< std::string >& _name_hash ) -> std::vector< cShared_ptr< Asset_t > >;

		template< class Ty >
		enable_if_asset_t< Ty, Asset_id_t >
		registerAsset( const cShared_ptr< Ty >& _asset ){ return registerAsset( _asset ); } // registerAsset

		Asset_id_t registerAsset( const cShared_ptr< Asset_t >& _asset );

		void loadFile( const std::filesystem::path& _path );

		static auto getAbsolutePath ( const std::filesystem::path& _path ) -> std::filesystem::path;
		static void makeAbsolutePath(       std::filesystem::path& _path );

	private:
		typedef std::vector< cShared_ptr< Asset_t > >( *load_file_func_t )( const std::filesystem::path& );
		typedef std::unordered_map< hash< Asset_id_t >, cShared_ptr< Asset_t > > id_to_asset_map_t;
		typedef std::unordered_multimap< hash< std::string >, cShared_ptr< Asset_t > > name_to_asset_map_t;
		typedef std::unordered_map< hash< std::string >, load_file_func_t > extension_loader_map_t;
		typedef extension_loader_map_t::value_type extension_map_entry_t;

#define EXTENSION_ENTRY( Ext, Func ) extension_map_entry_t{ hash< std::string >( Ext ), Func },

		static auto  loadGltfFile     ( const std::filesystem::path& _path ) -> std::vector< cShared_ptr< Asset_t > >;
		static auto  handleGltfMesh   ( const fastgltf::Asset& _asset, fastgltf::Mesh&    _mesh    ) -> cShared_ptr< Asset_t >;
		static auto  handleGltfTexture( const fastgltf::Asset& _asset, fastgltf::Texture& _texture ) -> cShared_ptr< Asset_t >;

		void loadEmbedded( void );

		extension_loader_map_t m_load_callbacks
		{
			EXTENSION_ENTRY( ".glb",  loadGltfFile )
			EXTENSION_ENTRY( ".gltf", loadGltfFile )
		};

		id_to_asset_map_t   m_assets;
		name_to_asset_map_t m_asset_name_map;

	};

	namespace Assets
	{
		template< class Ty, class... Args >
		cShared_ptr< Ty > create( Args... _args ){ auto asset = qw::cShared_ptr< Ty >( QW_NEW( Ty, 1, _args... ) ); cAssetManager::get().registerAsset( asset ); return asset; }
	} // Assets::

} // qw::
