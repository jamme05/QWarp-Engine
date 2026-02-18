/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */


#pragma once

#include <sk/Assets/Asset.h>
#include <sk/Assets/Access/Asset_Ptr.h>
#include <sk/Assets/Access/Asset_Ref.h>
#include <sk/Assets/Utils/Asset_List.h>
#include <sk/Containers/Map.h>
#include <sk/Misc/Singleton.h>
#include <sk/Misc/Smart_Ptrs.h>

#include <fastgltf/core.hpp>

#include <unordered_set>

namespace sk
{
	class cAsset_Meta;

	namespace Assets
	{
		class cAsset_List;

		enum class eGltfFilter
		{
			kScene,
			kModel,
			kTexture,
		};
		
		enum class eAssetTask : uint8_t
		{
			kLoadMeta,
			kLoadAsset,
			kRefreshAsset,
			kUnloadAsset,
			kSaveAsset,
		};
	}  // Assets

	class cAsset_Manager : public cSingleton< cAsset_Manager >
	{
		friend class cAsset_Meta;
	public:
		using id_to_asset_map_t = unordered_map< hash< cUUID >, cShared_ptr< cAsset_Meta > >;
		using file_pair_t       = std::pair< str_hash, std::string >;

		 cAsset_Manager();
		~cAsset_Manager() override;

		auto GetAllAssets() const -> const id_to_asset_map_t&;

		auto getAsset( const cUUID _id ) -> cShared_ptr< cAsset_Meta >;

		template< class Ty >
		requires std::is_base_of_v< cAsset_Meta, Ty >
		[[ deprecated( "With the changes to the asset system this no longer works." ) ]]
		auto getAssetAs( const cUUID& _id ) -> cShared_ptr< Ty >
		{
			if( auto asset = getAsset( _id ); asset && asset->GetClass()->isDerivedFrom( Ty::GetStaticClass() ) )
				return asset.Cast< Ty >();

			return nullptr;
		} // getAsset

		// Gets the first asset by name.
		auto getAssetByName ( const str_hash& _name_hash ) -> cShared_ptr< cAsset_Meta >;
		// Gets all assets by name.
		auto getAssetsByName( const str_hash& _name_hash ) -> Assets::cAsset_List;

		// Gets the first asset by path.
		auto GetAssetByPath ( const std::filesystem::path& _path ) -> cShared_ptr< cAsset_Meta >;
		// Gets all assets made from the same file. ( Multiple in the case of packs )
		auto GetAssetsByPath( const std::filesystem::path& _path ) -> Assets::cAsset_List;
		// Gets the first asset by path. Will not try to fix your path.
		// DONT USE THIS
		auto GetAssetByPathHash ( const cStringID& _path_hash ) -> cShared_ptr< cAsset_Meta >;
		// Gets all assets made from the same file. ( Multiple in the case of packs ). Will not try to fix your path.
		auto GetAssetsByPathHash( const cStringID& _path_hash ) -> Assets::cAsset_List;

		 /**
		  * 
		  * @param _asset  The asset to register.
		  * @param _reload Tells the asset manager if it should replace the previous version of the asset with this.
		  * @return The asset ID of said asset.
		  */
		cUUID registerAsset( const cShared_ptr< cAsset_Meta >& _asset, const bool _reload = false );

		 /**
		  * 
		  * @param _path      The path to the folder.
		  * @param _recursive 
		  * @return
		  */
		auto loadFolder( const std::filesystem::path& _path, const bool _recursive = true ) -> Assets::cAsset_List;
		auto loadFile  ( const std::filesystem::path& _path ) -> Assets::cAsset_List;

		// Asset ptrs
		template< class Ty >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetPtrByName ( const str_hash& _name_hash, const cShared_ptr< iClass >& _self = nullptr, const bool _load_asset = true )
			-> cAsset_Ptr< Ty >
		{
			if( const auto asset = getAssetByName( _name_hash ) )
				return GetAssetPtr< Ty >( asset, _self, _load_asset );
			return cAsset_Ptr< Ty >( _self );
		}
		template< class Ty >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetPtrByPath ( const cStringID& _path, const cShared_ptr< iClass >& _self = nullptr, const bool _load_asset = true )
			-> cAsset_Ptr< Ty >
		{
			if( const auto asset = GetAssetByPathHash( _path ) )
				return GetAssetPtr< Ty >( asset, _self, _load_asset );
			return cAsset_Ptr< Ty >( _self );
		}
		template< class Ty >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetPtrById   ( const cUUID& _uuid, const cShared_ptr< iClass >& _self = nullptr, const bool _load_asset = true )
			-> cAsset_Ptr< Ty >
		{
			if( const auto asset = getAsset( _uuid ) )
				return GetAssetPtr< Ty >( asset, _self, _load_asset );
			return cAsset_Ptr< Ty >( _self );
		}
		template< class Ty >
		requires std::is_base_of_v< cAsset, Ty >
		static auto GetAssetPtr( const cShared_ptr< cAsset_Meta >& _meta, const cShared_ptr< iClass >& _self = nullptr, const bool _load_asset = true )
			-> cAsset_Ptr< Ty >
		{
			auto ptr = cAsset_Ptr< Ty >{ _self, _meta };
			if( _load_asset )
				ptr.LoadSync();
		
			return ptr;
		}

		// Asset Refs
		template< class Ty, eAsset_Ref_Mode Mode = eAsset_Ref_Mode::kAutomaticAsync >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetRefByName ( const str_hash& _name_hash, const cShared_ptr< iClass >& _self = nullptr )
			-> cAsset_Ref< Ty, Mode >
		{
			if( const auto asset = getAssetByName( _name_hash ) )
				return GetAssetRef< Ty, Mode >( asset, _self );
			return cAsset_Ref< Ty, Mode >( _self );
		}
		template< class Ty, eAsset_Ref_Mode Mode = eAsset_Ref_Mode::kAutomaticAsync >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetRefByPath ( const cStringID& _path, const cShared_ptr< iClass >& _self = nullptr )
			-> cAsset_Ref< Ty, Mode >
		{
			if( const auto asset = GetAssetByPathHash( _path ) )
				return GetAssetRef< Ty, Mode >( asset, _self );
			return cAsset_Ref< Ty, Mode >( _self );
		}
		template< class Ty, eAsset_Ref_Mode Mode = eAsset_Ref_Mode::kAutomaticAsync >
		requires std::is_base_of_v< cAsset, Ty >
		auto GetAssetRefById   ( const cUUID& _uuid, const cShared_ptr< iClass >& _self = nullptr )
			-> cAsset_Ref< Ty, Mode >
		{
			if( const auto asset = getAsset( _uuid ) )
				return GetAssetRef< Ty, Mode >( asset, _self );
			return cAsset_Ref< Ty, Mode >( _self );
		}
		template< class Ty, eAsset_Ref_Mode Mode = eAsset_Ref_Mode::kAutomaticAsync >
		requires std::is_base_of_v< cAsset, Ty >
		static auto GetAssetRef( const cShared_ptr< cAsset_Meta >& _meta, const cShared_ptr< iClass >& _self = nullptr )
			-> cAsset_Ref< Ty, Mode >
		{
			return cAsset_Ref< Ty, Mode >{ _self, _meta };
		}
		
		template< class Ty, class... Args >
		requires ( std::is_base_of_v< cAsset, Ty > && std::constructible_from< Ty, Args... > )
		auto CreateAsset( std::string_view _name, const std::filesystem::path& _path, Args&&... _args ) -> std::pair< cShared_ptr< cAsset_Meta >, Ty* >
		{
			// TODO: Remove the need to specify the full path.
			cShared_ptr< cAsset_Meta > meta = sk::MakeShared< cAsset_Meta >( _name, kTypeInfo< Ty > );
			auto asset = SK_SINGLE( Ty, std::forward< Args >( _args )... );
			meta->setAsset( asset );
			meta->setPath( getAbsolutePath( _path ) );
			
			meta->m_flags_ |= cAsset_Meta::eFlags::kManualCreation | cAsset_Meta::eFlags::kLoaded;
			
			registerAsset( meta );
			
			return std::make_pair( meta, asset );
		}

		static auto getAbsolutePath ( const std::filesystem::path& _path ) -> std::filesystem::path;
		static void makeAbsolutePath(       std::filesystem::path& _path );

		using load_file_func_t = std::function< void( const std::filesystem::path&, Assets::cAsset_List&, Assets::eAssetTask ) >;

		void AddFileLoaderForExtensions( const std::vector< cStringID >& _extensions, const load_file_func_t& _function );
		void AddFileLoaderForExtension ( const cStringID& _extension, const load_file_func_t& _function );
		void RemoveFileLoaders( const std::vector< cStringID >& _extensions );
		auto GetFileLoader   ( const str_hash& _extension_hash ) -> load_file_func_t;
		auto GetExtensions   () -> std::vector< cStringID >;
	
	private:
		struct sRef_Info
		{
			using referrer_set_t = std::unordered_multiset< const void* >;
			referrer_set_t referrers;
			
		};

		using str_to_asset_map_t     = unordered_multimap< str_hash, cShared_ptr< cAsset_Meta > >;
		using path_to_ref_map_t      = unordered_map< str_hash, sRef_Info >;
		using extension_loader_map_t = unordered_map< cStringID, load_file_func_t >;
		using extension_map_entry_t  = extension_loader_map_t::value_type;

		void addPathReferrer   ( const str_hash& _path_hash, const void* _referrer );
		// Returns if there are no more referrers.
		bool removePathReferrer( const str_hash& _path_hash, const void* _referrer );
		
		static void loadGltfFile         ( const std::filesystem::path& _path, Assets::cAsset_List& _metas, Assets::eAssetTask _load_task );
		static auto createGltfMeshMeta   ( const fastgltf::Mesh& _mesh, size_t _index ) -> cShared_ptr< cAsset_Meta >;
		static auto createGltfTextureMeta( const fastgltf::Texture& _texture, size_t _index ) -> cShared_ptr< cAsset_Meta >;
		static void handleGltfMesh       ( cAsset_Meta& _meta, const fastgltf::Asset& _asset, fastgltf::Mesh& _mesh, Assets::eAssetTask _task );
		static void handleGltfTexture    ( cAsset_Meta& _meta, const fastgltf::Asset& _asset, fastgltf::Texture& _texture, Assets::eAssetTask _task );

		static void loadPngFile      ( const std::filesystem::path& _path, Assets::cAsset_List& _assets, Assets::eAssetTask _load_task );

		static auto tryLoadAssetMetaFile( std::filesystem::path _path ) -> cShared_ptr< cAsset_Meta >;
		
		void loadEmbedded( void );

#define EXTENSION_ENTRY( Ext, Func ) extension_map_entry_t{ cStringID( Ext ), Func },

		// It may show an error but is perfectly fine.
		extension_loader_map_t m_load_callbacks_
		{
			EXTENSION_ENTRY( "glb",    loadGltfFile )
			EXTENSION_ENTRY( "gltf",   loadGltfFile )
			EXTENSION_ENTRY( "png",    loadPngFile  )
		};

		id_to_asset_map_t  m_assets_;
		str_to_asset_map_t m_asset_name_map_;
		str_to_asset_map_t m_asset_path_map_;
		path_to_ref_map_t  m_path_ref_map_;
	};

	namespace Assets
	{
		template< class Ty, class... Args >
		cShared_ptr< Ty > create( Args... _args ){ auto asset = sk::cShared_ptr< Ty >( SK_NEW( Ty, 1, _args... ) ); cAsset_Manager::get().registerAsset( asset ); return asset; }
	} // Assets::

} // sk::
