/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "Asset_Manager.h"

#include <sk/Assets/Mesh.h>
#include <sk/Assets/Model.h>
#include <sk/Assets/Texture.h>
#include <sk/Assets/Management/Asset_Job_Manager.h>
#include <sk/Assets/Utils/Asset_List.h>
#include <sk/Graphics/Buffer/Dynamic_Buffer.h>
#include <sk/Scene/Managers/CameraManager.h>

#include <fastgltf/tools.hpp>

#include <any>

namespace sk
{
	cAsset_Manager::cAsset_Manager()
	{
		std::filesystem::current_path( std::filesystem::path{ SK_ROOT_DIR }.make_preferred() );

		loadEmbedded();

		Assets::Jobs::cAsset_Job_Manager::init();
	} // cAssetManager

	cAsset_Manager::~cAsset_Manager()
	{
		auto& job_manager = Assets::Jobs::cAsset_Job_Manager::get();
		
		job_manager.m_shutting_down_.store( true );
		Assets::Jobs::sTask task;
		task.type = Assets::Jobs::eJobType::kUnload;
		
		std::unordered_set< str_hash > queued;
		for( auto& path_hash : m_asset_path_map_ | std::views::keys )
		{
			if( queued.contains( path_hash ) )
				continue;
			
			queued.insert( path_hash );
			
			Assets::cAsset_List assets;
			for( auto [ fst, lst ] = m_asset_path_map_.equal_range( path_hash ); fst != lst; ++fst )
				assets.AddAsset( fst->second );

			// It's ugly but it works
			// TODO: Use stringID instead of str_hash in places like these.
			const auto path   = assets.begin()->GetAbsolutePath();
			const auto loader = GetFileLoader( assets.begin()->m_ext_ );
			
			if( loader == nullptr )
			{
				for( auto& asset : assets )
					asset->setAsset( nullptr );
				continue;
			}
			
			task.data = ::new( Memory::alloc_fast( sizeof( Assets::Jobs::sAssetTask ) ) ) Assets::Jobs::sAssetTask{
				.path    = path.view(), 
				.affected_assets = assets,
				.loader       = loader,
			};
			
			job_manager.push_task( task );
		}
		for( auto& asset : m_assets_ | std::views::values )
			asset->setAsset( nullptr );
		Assets::Jobs::cAsset_Job_Manager::shutdown();
		
		m_assets_.clear();
		m_asset_path_map_.clear();
		m_asset_name_map_.clear();
		m_path_ref_map_.clear();
	} // ~cAssetManager

	auto cAsset_Manager::GetAllAssets() const -> const id_to_asset_map_t&
	{
		return m_assets_;
	}

	auto cAsset_Manager::getAsset( const cUUID _id ) -> cShared_ptr< cAsset_Meta >
	{
		if( const auto asset_itr = m_assets_.find( _id ); asset_itr != m_assets_.end() )
			return asset_itr->second;

		return nullptr;
	} // getAsset

	auto cAsset_Manager::getAssetByName( const str_hash& _name_hash ) -> cShared_ptr< cAsset_Meta >
	{
		if( const auto itr = m_asset_name_map_.find( _name_hash ); itr != m_asset_name_map_.end() )
			return itr->second;

		return nullptr;
	} // getAssetByName
	auto cAsset_Manager::getAssetsByName( const str_hash& _name_hash ) -> Assets::cAsset_List
	{
		Assets::cAsset_List assets;
		const auto range = m_asset_name_map_.equal_range( _name_hash );
		for( auto it = range.first; it != range.second; ++it )
			assets.AddAsset( it->second );

		return assets;
	} // getAssetsByName

	auto cAsset_Manager::GetAssetByPath( const std::filesystem::path& _path ) -> cShared_ptr< cAsset_Meta >
	{
		if( _path.is_relative() )
			return GetAssetByPathHash( std::string_view{ getAbsolutePath( _path ).string() } );

		return GetAssetByPathHash( std::string_view{ _path.string() } );
	} // getAssetByPath

	auto cAsset_Manager::GetAssetsByPath( const std::filesystem::path& _path ) -> Assets::cAsset_List
	{
		if( _path.is_relative() )
			return GetAssetsByPathHash( std::string_view{ getAbsolutePath( _path ).string() } );

		return GetAssetsByPathHash( std::string_view{ _path.string() } );
	} // getAssetsByPath
	
	auto cAsset_Manager::GetAssetByPathHash( const cStringID& _path_hash ) -> cShared_ptr< cAsset_Meta >
	{
		// TODO: Decide if we're going to correct the path here.
		if( const auto path = std::filesystem::path{ _path_hash.view() }; path.is_relative() )
			return GetAssetByPath( path );

		if( const auto itr = m_asset_path_map_.find( _path_hash ); itr != m_asset_path_map_.end() )
			return itr->second;

		return nullptr;
	}

	auto cAsset_Manager::GetAssetsByPathHash( const cStringID& _path_hash ) -> Assets::cAsset_List
	{
		// TODO: Decide if we're going to correct the path here.
		if( const auto path = std::filesystem::path{ _path_hash.view() }; path.is_relative() )
			return GetAssetsByPath( path );

		Assets::cAsset_List assets;
		for( auto [ fst, lst ] = m_asset_path_map_.equal_range( _path_hash ); fst != lst; ++fst )
			assets.AddAsset( fst->second );

		return assets;
	}

	cUUID cAsset_Manager::registerAsset( const cShared_ptr< cAsset_Meta >& _asset, const bool _reload )
	{
		auto& id = _asset->m_uuid_;
		// TODO: Add check if asset already exists.
		if( id == cUUID::kInvalid )
			id = GenerateRandomUUID();
		if( !_reload )
		{
			// New asset, provide an uuid to it and register it.
			m_assets_[ id ] = _asset;
			m_asset_name_map_.insert( { _asset->GetName().hash(),  _asset } );
			m_asset_path_map_.insert( { _asset->GetAbsolutePath(), _asset } );
		}
		else
			_asset->Reload();

		return _asset->m_uuid_;
	} // registerAsset

	auto cAsset_Manager::loadFolder( const std::filesystem::path& _path, const bool _recursive ) -> Assets::cAsset_List
	{
		Assets::cAsset_List assets;

		// TODO: Implament loading folders.
		if( _recursive )
		{
			std::filesystem::recursive_directory_iterator iter( _path );
			for( auto& file : iter )
			{
				auto ext = file.path().extension().string();
				// Look into making it safe. https://en.cppreference.com/w/cpp/filesystem/directory_entry.html
				if( file.is_regular_file() && file.path().has_extension()
					&& m_load_callbacks_.contains( std::string_view{ ext.data() + 1, ext.length() - 1 } ) )
					assets += loadFile( file.path() );
			}
		}
		else
		{
			std::filesystem::directory_entry iter( _path );
		}

		return assets;
	} // loadFolder

	auto cAsset_Manager::loadFile( const std::filesystem::path& _path ) -> Assets::cAsset_List
	{
		auto ext = _path.extension().string();
		SK_ERR_IF( ext.empty(),
			"Error: File extension is empty." )

		const auto absolute_path = getAbsolutePath( _path );

		if( auto asset = tryLoadAssetMetaFile( absolute_path ) )
		{
			asset->setPath( absolute_path );
			registerAsset( asset );

			auto list = Assets::cAsset_List{};
			list.AddAsset( asset );
			return list;
		}

		if( m_asset_path_map_.contains( absolute_path ) )
			return GetAssetsByPathHash( cStringID{ absolute_path.string() } );
		
		ext = ext.substr( 1 );
		
		const auto extension = cStringID( ext );

		const auto callback_pair = m_load_callbacks_.find( extension );

		if( callback_pair == m_load_callbacks_.end() )
			return {};

		Assets::cAsset_List assets;
		callback_pair->second( absolute_path, assets, Assets::eAssetTask::kLoadMeta );

		for( auto& asset : assets )
		{
			asset->setPath( absolute_path );
			registerAsset( asset );
		}

		return assets;
	} // loadFile

	auto cAsset_Manager::getAbsolutePath( const std::filesystem::path& _path ) -> std::filesystem::path
	{
		return ( std::filesystem::current_path() /= _path ).make_preferred();
	} // getAbsolutePath

	void cAsset_Manager::makeAbsolutePath( std::filesystem::path& _path )
	{
		_path = getAbsolutePath( _path );
	} // makeAbsolutePath

	void cAsset_Manager::AddFileLoaderForExtensions( const std::vector< cStringID >& _extensions, const load_file_func_t& _function )
	{
		for( const auto& extension : _extensions )
			AddFileLoaderForExtension( extension, _function );
	} // AddFileLoader
	
	void cAsset_Manager::AddFileLoaderForExtension( const cStringID& _extension, const load_file_func_t& _function )
	{
		SK_WARN_IF_RET( sk::Severity::kEngine, m_load_callbacks_.contains( _extension ),
			TEXT( "Warning: Asset manager already contains a loader for {}", _extension.view() ) )
		
		m_load_callbacks_.emplace( _extension, _function );
	}

	void cAsset_Manager::RemoveFileLoaders( const std::vector< cStringID >& _extensions )
	{
		for( const auto& extension : _extensions )
			m_load_callbacks_.erase( extension );
	}

	auto cAsset_Manager::GetFileLoader( const str_hash& _extension_hash ) -> load_file_func_t
	{
		if( const auto itr = m_load_callbacks_.find( cStringID::FromHash( _extension_hash ) ); itr != m_load_callbacks_.end() )
			return itr->second;
		return nullptr;
	}

	auto cAsset_Manager::GetExtensions() -> std::vector< cStringID >
	{
		static std::vector< cStringID > extensions;
		extensions.clear();
		extensions.reserve( m_load_callbacks_.size() );
		
		for( auto& extension : m_load_callbacks_ | std::views::keys )
			extensions.emplace_back( extension );
		
		return extensions;
	}

	void cAsset_Manager::addPathReferrer( const str_hash& _path_hash, const void* _referrer )
	{
		auto& [ referrers ] = m_path_ref_map_[ _path_hash ];
		referrers.emplace( _referrer );
		
		auto  [ fst, lst ] = m_asset_path_map_.equal_range( _path_hash );

		// TODO: Remember what I was planning here.
	}

	bool cAsset_Manager::removePathReferrer( const str_hash& _path_hash, const void* _referrer )
	{
		const auto itr = m_path_ref_map_.find( _path_hash );
		if( itr == m_path_ref_map_.end() )
			return false;

		auto& referrers = itr->second.referrers;
		referrers.erase( referrers.find( _referrer ) );
		
		return referrers.empty();
	}

	void cAsset_Manager::loadGltfFile( const std::filesystem::path& _path, Assets::cAsset_List& _metas, Assets::eAssetTask _load_task )
	{
		if( _load_task == Assets::eAssetTask::kUnloadAsset || _load_task == Assets::eAssetTask::kSaveAsset )
			return;
		
		// TODO: Cache the result from this gltf file for each individual asset.

		fastgltf::Parser parser;

		auto data = fastgltf::GltfDataBuffer::FromPath( _path );
		if( data.error() != fastgltf::Error::None )
			return;

		auto raw_asset = parser.loadGltf( data.get(), _path, fastgltf::Options::GenerateMeshIndices );
		if( auto error = raw_asset.error(); error != fastgltf::Error::None )
			return;

		auto& asset = raw_asset.get();

		for( auto& node : asset.nodes )
		{
			// TODO: Models
			// TODO: Prefabs?
			// TODO: Decide if models are gonna be a thing, or just have them be prefabs.
		}
		
		auto [ tex_fst, tex_lst ] = _metas.GetRange< Assets::cTexture >();
		for( size_t i = 0; i < asset.textures.size(); i++ )
		{
			if( _load_task == Assets::eAssetTask::kLoadMeta )
			{
				auto& texture = asset.textures[ i ];
				_metas.AddAsset( createGltfTextureMeta( texture, i ) );
			}
			else
			{
				auto& meta    = *tex_fst->second;
				auto& texture = asset.textures[ meta.GetStoreIndex() ];
				handleGltfTexture( meta, asset, texture, _load_task );
				++tex_fst;
			}
		}
		
		auto [ mesh_fst, mesh_lst ] = _metas.GetRange< Assets::cMesh >();
		for( size_t i = 0; i < asset.meshes.size(); i++ )
		{
			if( _load_task == Assets::eAssetTask::kLoadMeta )
			{
				auto& mesh = asset.meshes[ i ];
				_metas.AddAsset( createGltfMeshMeta( mesh, i ) );
			}
			else
			{
				auto& meta = *mesh_fst->second;
				auto& mesh = asset.meshes[ meta.GetStoreIndex() ];
				handleGltfMesh( meta, asset, mesh, _load_task );
				++mesh_fst;
			}
		}

		if( _load_task == Assets::eAssetTask::kLoadMeta )
		{
			for( auto& meta : _metas )
				meta->m_flags_ |= cAsset_Meta::eFlags::kSharesPath;
		}
	} // loadGltfFile
	
	auto cAsset_Manager::createGltfMeshMeta( const fastgltf::Mesh& _mesh, const size_t _index ) -> cShared_ptr< cAsset_Meta >
	{
		auto meta = sk::MakeShared< cAsset_Meta >( std::string_view( _mesh.name ), &sk::kTypeInfo< Assets::cMesh > );
		meta->SetStoreIndex( _index );
		
		return meta;
	}

	auto cAsset_Manager::createGltfTextureMeta( const fastgltf::Texture& _texture,
		size_t _index )->cShared_ptr< cAsset_Meta >
	{
		auto meta = sk::MakeShared< cAsset_Meta >( std::string_view( _texture.name ), &sk::kTypeInfo< Assets::cTexture > );
		meta->SetStoreIndex( _index );
		
		return meta;
	}

	namespace 
	{
		auto get_accessor_source_bytes( const fastgltf::Asset& _asset, const fastgltf::Accessor& _accessor )
		{
			constexpr fastgltf::DefaultBufferDataAdapter adapter;
			
			return adapter( _asset, *_accessor.bufferViewIndex ).subspan( _accessor.byteOffset );
		}

		void fill_index_buffer( Assets::cMesh& _mesh, const fastgltf::Asset& _asset, const fastgltf::Accessor& _accessor )
		{
			auto& index_buffer = _mesh.GetIndexBuffer();
			
			switch( _accessor.componentType )
			{
			case fastgltf::ComponentType::UnsignedByte:
				_mesh.CreateIndexBufferFrom( Assets::cMesh::eIndexType::k16, nullptr, _accessor.count );
				fastgltf::copyFromAccessor< uint16_t >( _asset, _accessor, index_buffer->RawData() );
			break;
			case fastgltf::ComponentType::UnsignedShort:
			{
				const auto data = get_accessor_source_bytes( _asset, _accessor );
				_mesh.CreateIndexBufferFrom( Assets::cMesh::eIndexType::k16, data.data(), _accessor.count );
			}
			break;
			case fastgltf::ComponentType::UnsignedInt:
			{
				const auto data = get_accessor_source_bytes( _asset, _accessor );
				_mesh.CreateIndexBufferFrom( Assets::cMesh::eIndexType::k32, data.data(), _accessor.count );
			}
			break;
			default:
				SK_FATAL( "What the fuck" )
			}
		}

		void fill_vertex_buffer( Graphics::cDynamic_Buffer& _buffer, const fastgltf::Asset& _asset, const fastgltf::Accessor& _accessor )
		{
			const auto data = get_accessor_source_bytes( _asset, _accessor );

			memcpy( _buffer.RawData(), data.data(), data.size() );
		}

		template< size_t R, size_t C = 1 >
		type_info_t get_accessor_type( const fastgltf::ComponentType _type )
		{
			if constexpr( C > 1 )
			{
				// Matrices
				switch( _type )
				{
				case fastgltf::ComponentType::Invalid:       return nullptr;
				case fastgltf::ComponentType::Byte:          return &kTypeInfo< Math::cMatrix< R, C, int8_t > >;
				case fastgltf::ComponentType::UnsignedByte:  return &kTypeInfo< Math::cMatrix< R, C, uint8_t > >;
				case fastgltf::ComponentType::Short:         return &kTypeInfo< Math::cMatrix< R, C, int16_t > >;
				case fastgltf::ComponentType::UnsignedShort: return &kTypeInfo< Math::cMatrix< R, C, uint16_t > >;
				case fastgltf::ComponentType::Int:           return &kTypeInfo< Math::cMatrix< R, C, int32_t > >;
				case fastgltf::ComponentType::UnsignedInt:   return &kTypeInfo< Math::cMatrix< R, C, uint32_t > >;
				case fastgltf::ComponentType::Float:         return &kTypeInfo< Math::cMatrix< R, C, float > >;
				case fastgltf::ComponentType::Double:        return &kTypeInfo< Math::cMatrix< R, C, double > >;
				}
			}
			else if constexpr( R == 1 )
			{
				// Scalar
				switch( _type )
				{
				case fastgltf::ComponentType::Invalid:       return nullptr;
				case fastgltf::ComponentType::Byte:          return &kTypeInfo< int8_t >;
				case fastgltf::ComponentType::UnsignedByte:  return &kTypeInfo< uint8_t >;
				case fastgltf::ComponentType::Short:         return &kTypeInfo< int16_t >;
				case fastgltf::ComponentType::UnsignedShort: return &kTypeInfo< uint16_t >;
				case fastgltf::ComponentType::Int:           return &kTypeInfo< int32_t >;
				case fastgltf::ComponentType::UnsignedInt:   return &kTypeInfo< uint32_t >;
				case fastgltf::ComponentType::Float:         return &kTypeInfo< float >;
				case fastgltf::ComponentType::Double:        return &kTypeInfo< double >;
				}
			}
			else
			{
				// Vectors
				switch( _type )
				{
				case fastgltf::ComponentType::Invalid:       return nullptr;
				case fastgltf::ComponentType::Byte:          return &kTypeInfo< Math::cVector< R, int8_t > >;
				case fastgltf::ComponentType::UnsignedByte:  return &kTypeInfo< Math::cVector< R, uint8_t > >;
				case fastgltf::ComponentType::Short:         return &kTypeInfo< Math::cVector< R, int16_t > >;
				case fastgltf::ComponentType::UnsignedShort: return &kTypeInfo< Math::cVector< R, uint16_t > >;
				case fastgltf::ComponentType::Int:           return &kTypeInfo< Math::cVector< R, int32_t > >;
				case fastgltf::ComponentType::UnsignedInt:   return &kTypeInfo< Math::cVector< R, uint32_t > >;
				case fastgltf::ComponentType::Float:         return &kTypeInfo< Math::cVector< R, float > >;
				case fastgltf::ComponentType::Double:        return &kTypeInfo< Math::cVector< R, double > >;
				}
			}

			return nullptr;
		}

		type_info_t get_accessor_type( const fastgltf::Accessor& _accessor )
		{
			switch( _accessor.type )
			{
			case fastgltf::AccessorType::Invalid: return nullptr;
			case fastgltf::AccessorType::Scalar:  return get_accessor_type< 1 >( _accessor.componentType );
			case fastgltf::AccessorType::Vec2:    return get_accessor_type< 2 >( _accessor.componentType );
			case fastgltf::AccessorType::Vec3:    return get_accessor_type< 3 >( _accessor.componentType );
			case fastgltf::AccessorType::Vec4:    return get_accessor_type< 4 >( _accessor.componentType );
			case fastgltf::AccessorType::Mat2:    return get_accessor_type< 2, 2 >( _accessor.componentType );
			case fastgltf::AccessorType::Mat3:    return get_accessor_type< 3, 3 >( _accessor.componentType );
			case fastgltf::AccessorType::Mat4:    return get_accessor_type< 4, 4 >( _accessor.componentType );
			}

			return nullptr;
		}

		void fill_vertex_buffers( Assets::cMesh& _mesh, const fastgltf::Asset& _asset, const fastgltf::Attribute* _attributes, const size_t _attribute_count )
		{
			auto& vertex_buffers = _mesh.GetVertexBuffers();
			
			for( size_t i = 0; i < _attribute_count; i++ )
			{
				const auto& [ pmr_name, accessorIndex ] = _attributes[ i ];

				auto& accessor = _asset.accessors[ accessorIndex ];

				auto buffer = sk::MakeShared< Graphics::cDynamic_Buffer >(
					std::format( "{}: {}", _mesh.GetName(), std::string_view{ pmr_name } ),
					Graphics::Buffer::eType::kVertex, accessor.normalized
				);

				cStringID name = std::string_view{ pmr_name }; 

				buffer->AlignAs( get_accessor_type( accessor ), false );
				buffer->Resize( accessor.count );
				
				fill_vertex_buffer( *buffer, _asset, accessor );

				vertex_buffers.emplace( name, buffer );

				switch( name.hash() )
				{
				case str_hash( "POSITION" ):
				{
					static constexpr cStringID kAliases[] = { "Position", "aPosition" };
					for( auto& alias : kAliases )
						vertex_buffers.emplace( alias, buffer );
				}
				break;
				case str_hash( "NORMAL" ):
				{
					static constexpr cStringID kAliases[] = { "Normal", "aNormal" };
					for( auto& alias : kAliases )
						vertex_buffers.emplace( alias, buffer );
				}
				break;
				case str_hash( "TEXCOORD_0" ):
				{
					static constexpr cStringID kAliases[] = { "TexCoord", "aTexCoord", "UV", "aUV" };
					for( auto& alias : kAliases )
						vertex_buffers.emplace( alias, buffer );
				}
				break;
				default: break;
				}
			}
		}
	} // ::

	void cAsset_Manager::handleGltfMesh( cAsset_Meta& _meta, const fastgltf::Asset& _asset, fastgltf::Mesh& _mesh, const Assets::eAssetTask _task )
	{
		const auto mesh_asset = sk::Memory::alloc< Assets::cMesh >( 1, std::source_location::current() , _meta.GetName().string() );
		
		for( auto& primitive : _mesh.primitives )
		{
			// Check if primitive has indices, go to next if it doesn't.
			if( !primitive.indicesAccessor.has_value() )
				continue;
			
			auto& accessor = _asset.accessors[ primitive.indicesAccessor.value() ];

			fill_index_buffer( *mesh_asset, _asset, accessor );

			fill_vertex_buffers( *mesh_asset, _asset, primitive.attributes.data(), primitive.attributes.size() );

			// TODO: Support multiple primitives
			break;
		} // auto& primitive : _mesh.primitives
		
		_meta.setAsset( mesh_asset );
	} // handleGltfMesh
	
	void cAsset_Manager::handleGltfTexture( cAsset_Meta& _meta, const fastgltf::Asset& _asset, fastgltf::Texture& _texture, const Assets::eAssetTask _task )
	{
		if( !_texture.imageIndex.has_value() )
			return;
		
		if( _task == Assets::eAssetTask::kLoadMeta )
			return;
		
		auto& image = _asset.images[ _texture.imageIndex.value() ];
		
		const uint8_t* target_buffer = nullptr;
		size_t         size;
		fastgltf::MimeType type;
		
		// TODO: Support external sources.

		std::visit(fastgltf::visitor {
			[](auto& arg) {},
			[&](const fastgltf::sources::URI& uri) {},
			[&](fastgltf::sources::Array& vector) {
				size = vector.bytes.size();
				target_buffer = reinterpret_cast< const uint8_t* >( vector.bytes.data() );
				type = vector.mimeType;
			},
			[&](const fastgltf::sources::BufferView& view) {
				auto& bufferView = _asset.bufferViews[view.bufferViewIndex];
				auto& buffer = _asset.buffers[bufferView.bufferIndex];
				// Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
				// to just copy the buffer data again for the texture. Besides, this is just an example.

				type = view.mimeType;

				if( const auto vector = std::get_if< fastgltf::sources::Array >( &buffer.data ) )
				{
					size = vector->bytes.size();
					target_buffer = reinterpret_cast< const uint8_t* >( vector->bytes.data() ) + bufferView.byteOffset;
				}
			},
		}, image.data);
		
		_meta.setAsset( SK_SINGLE( Assets::cTexture, std::string{ image.name }, target_buffer, size ) );
	} // handleGltfTexture

	void cAsset_Manager::loadPngFile( const std::filesystem::path& _path, Assets::cAsset_List& _assets, Assets::eAssetTask _load_task )
	{
		// TODO: Standalone Png file loader.
	} // loadPNGFile

	auto cAsset_Manager::tryLoadAssetMetaFile( std::filesystem::path _path ) -> cShared_ptr< cAsset_Meta >
	{
		if( _path.has_extension() )
			_path.replace_extension( "skmeta" );

		if( !std::filesystem::exists( _path ) )
			return nullptr;

		std::ifstream json_file{ _path, std::ifstream::in | std::ifstream::binary | std::ifstream::ate };
		json_file.seekg( 0, json_file.end );

		const size_t size = json_file.tellg();
		json_file.seekg( 0, json_file.beg );

		std::string json_str( size, 0 );
		json_file.read( json_str.data(), json_str.size() );

		simdjson::ondemand::parser parser;
		auto doc = parser.iterate( json_str );
		auto object = cSerializedObject{ doc.get_object() };
		auto meta = sk::MakeShared< cAsset_Meta >( object );

		return meta;
	}

	void cAsset_Manager::loadEmbedded( void )
	{
	} // loadEmbedded
} // sk::
