/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cAsset_manager.h"

#include "fastgltf/tools.hpp"

#include "Assets/cMesh.h"
#include "Assets/cTexture.h"
#include "Graphics/cRenderer.h"
#include "Platform/cPlatform.h"

namespace qw
{
	cAssetManager::cAssetManager( void )
	{
		loadEmbedded();
	} // cAssetManager

	cAssetManager::~cAssetManager( void )
	{
		m_assets.clear();
	} // ~cAssetManager

	auto cAssetManager::getAsset( const Asset_id_t _id ) -> cShared_ptr< Asset_t >
	{
		if( const auto asset_itr = m_assets.find( _id ); asset_itr != m_assets.end() )
			return asset_itr->second;

		return nullptr;
	} // getAsset

	auto cAssetManager::getAssetByName( const str_hash& _name_hash ) -> cShared_ptr< Asset_t >
	{
		if( const auto itr = m_asset_name_map.find( _name_hash ); itr != m_asset_name_map.end() )
			return itr->second;

		return nullptr;
	} // getAssetByName
	auto cAssetManager::getAssetsByName( const str_hash& _name_hash ) -> Assets::cAsset_List
	{
		Assets::cAsset_List assets;
		const auto range = m_asset_name_map.equal_range( _name_hash );
		for( auto it = range.first; it != range.second; ++it )
			assets.add_asset( it->second );

		return assets;
	} // getAssetsByName

	auto cAssetManager::getAssetByPath( const str_hash& _path_hash ) -> cShared_ptr< Asset_t >
	{
		if( const auto itr = m_asset_path_map.find( _path_hash ); itr != m_asset_path_map.end() )
			return itr->second;

		return nullptr;
	} // getAssetByPath

	auto cAssetManager::getAssetsByPath( const str_hash& _path_hash ) -> Assets::cAsset_List
	{
		Assets::cAsset_List assets;
		const auto range = m_asset_path_map.equal_range( _path_hash );
		for( auto it = range.first; it != range.second; ++it )
			assets.add_asset( it->second );

		return assets;
	} // getAssetsByPath

	Asset_id_t cAssetManager::registerAsset( const cShared_ptr< Asset_t >& _asset, const bool _reload )
	{
		// TODO: Add check if asset already exists.

		// TODO: Maybe make a new way to generate ids.
		const Asset_id_t id  = m_assets.size();
		m_assets[ id ]       = _asset;
		_asset->m_id         = id;
		m_asset_name_map.insert( { _asset->getNameHash(), _asset } );
		m_asset_path_map.insert( { _asset->getPath(),     _asset } );

		return id;
	} // registerAsset

	auto cAssetManager::loadFolder( const std::filesystem::path& _path, const bool _recursive, const bool _reload ) -> Assets::cAsset_List
	{
		Assets::cAsset_List assets;

		if( _recursive )
		{
			std::filesystem::recursive_directory_iterator iter( _path );
			for( const auto& file : iter )
			{
				assets += loadFile( file.path(), _reload );
			}
		}
		else
		{
			std::filesystem::directory_entry iter( _path );
		}

		return assets;
	} // loadFolder

	auto cAssetManager::loadFile( const std::filesystem::path& _path, const bool _reload ) -> Assets::cAsset_List
	{
		const auto extension_hash = str_hash( _path.extension() );

		const auto callback_pair = m_load_callbacks.find( extension_hash );

		if( callback_pair == m_load_callbacks.end() )
			return {};

		const auto absolute_path = getAbsolutePath( _path );
		const auto path_hash     = str_hash( absolute_path );

		auto assets = callback_pair->second( absolute_path );

		if( assets.empty() )
			return {};

		for( auto& asset : assets )
		{
			registerAsset( asset );
		}

		return assets;
	} // loadFile

	auto cAssetManager::getAbsolutePath( const std::filesystem::path& _path ) -> std::filesystem::path
	{
		return std::filesystem::path( OS_ROOT ) /= _path;
	} // getAbsolutePath

	void cAssetManager::makeAbsolutePath( std::filesystem::path& _path )
	{
		_path = getAbsolutePath( _path );
	} // makeAbsolutePath

	auto cAssetManager::loadGltfFile( const std::filesystem::path& _path ) -> Assets::cAsset_List
	{
		fastgltf::Parser parser;

		auto data = fastgltf::GltfDataBuffer::FromPath( _path );
		if( data.error() != fastgltf::Error::None )
			return {};

		auto raw_asset = parser.loadGltf( data.get(), _path.parent_path(), fastgltf::Options::None );
		if( auto error = raw_asset.error(); error != fastgltf::Error::None )
			return {};

		auto& asset = raw_asset.get();

		Assets::cAsset_List assets;

		for( auto& texture : asset.textures )
		{
			if( auto texture_asset = handleGltfTexture( asset, texture ) )
				assets.add_asset( texture_asset );
		}
		for( auto& mesh : asset.meshes )
		{
			if( auto mesh_asset = handleGltfMesh( asset, mesh ) )
				assets.add_asset( mesh_asset );
		}

		return assets;
	} // loadGltfFile

	auto cAssetManager::handleGltfMesh( const fastgltf::Asset& _asset, fastgltf::Mesh& _mesh ) -> cShared_ptr< Asset_t >
	{
		auto mesh_asset = Assets::cMesh::create_shared( std::string( _mesh.name ) );
		// TODO: Model asset where each primitive is a mesh asset
		for( auto& primitive : _mesh.primitives )
		{
			// Check if primitive has indices, go to next if it doesn't.
			if( !primitive.indicesAccessor.has_value() )
				continue;

			auto& index_accessor = _asset.accessors[ primitive.indicesAccessor.value() ];

			mesh_asset->loadIndicesFromAccessor( _asset, index_accessor );

			for( const auto& attribute : primitive.attributes )
			{
				auto  name_hash = str_hash( attribute.name );
				auto& buffers   = mesh_asset->m_buffers;

				auto& accessor  = _asset.accessors[ attribute.accessorIndex ];

				const size_t size = getElementByteSize( accessor.type, accessor.componentType ) * accessor.count;

				auto& buffer = buffers.m_buffers[ name_hash ];
				buffer.resize( size );

				//constexpr fastgltf::DefaultBufferDataAdapter adapter = {};

				// Copy raw?
				//auto src_bytes = adapter( _asset, *accessor.bufferViewIndex ).subspan( accessor.byteOffset );
				// TODO: Support more types

				// TODO: Replace this, check loadGltfTexture for reference on how to access data.

				switch( accessor.type )
				{
				case fastgltf::AccessorType::Scalar:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Byte:          fastgltf::copyFromAccessor< int8_t   >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedByte:  fastgltf::copyFromAccessor< uint8_t  >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Short:         fastgltf::copyFromAccessor< int16_t  >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedShort: fastgltf::copyFromAccessor< uint16_t >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Int:           fastgltf::copyFromAccessor< int32_t  >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedInt:   fastgltf::copyFromAccessor< uint32_t >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< float    >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Double:        fastgltf::copyFromAccessor< double   >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Invalid: break;
					}
				break;
				case fastgltf::AccessorType::Vec2:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Byte:          fastgltf::copyFromAccessor< fastgltf::math::vec< int8_t,   2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedByte:  fastgltf::copyFromAccessor< fastgltf::math::vec< uint8_t,  2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Short:         fastgltf::copyFromAccessor< fastgltf::math::vec< int16_t,  2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedShort: fastgltf::copyFromAccessor< fastgltf::math::vec< uint16_t, 2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Int:           fastgltf::copyFromAccessor< fastgltf::math::vec< int32_t,  2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedInt:   fastgltf::copyFromAccessor< fastgltf::math::vec< uint32_t, 2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::vec< float,    2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Double:        fastgltf::copyFromAccessor< fastgltf::math::vec< double,   2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Invalid: break;
					}
				break;
				case fastgltf::AccessorType::Vec3:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Byte:          fastgltf::copyFromAccessor< fastgltf::math::vec< int8_t,   3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedByte:  fastgltf::copyFromAccessor< fastgltf::math::vec< uint8_t,  3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Short:         fastgltf::copyFromAccessor< fastgltf::math::vec< int16_t,  3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedShort: fastgltf::copyFromAccessor< fastgltf::math::vec< uint16_t, 3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Int:           fastgltf::copyFromAccessor< fastgltf::math::vec< int32_t,  3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedInt:   fastgltf::copyFromAccessor< fastgltf::math::vec< uint32_t, 3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::vec< float,    3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Double:        fastgltf::copyFromAccessor< fastgltf::math::vec< double,   3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Invalid: break;
					}
				break;
				case fastgltf::AccessorType::Vec4:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Byte:          fastgltf::copyFromAccessor< fastgltf::math::vec< int8_t,   4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedByte:  fastgltf::copyFromAccessor< fastgltf::math::vec< uint8_t,  4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Short:         fastgltf::copyFromAccessor< fastgltf::math::vec< int16_t,  4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedShort: fastgltf::copyFromAccessor< fastgltf::math::vec< uint16_t, 4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Int:           fastgltf::copyFromAccessor< fastgltf::math::vec< int32_t,  4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::UnsignedInt:   fastgltf::copyFromAccessor< fastgltf::math::vec< uint32_t, 4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::vec< float,    4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Double:        fastgltf::copyFromAccessor< fastgltf::math::vec< double,   4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Invalid: break;
					}
				break;
				case fastgltf::AccessorType::Mat2:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::mat< float, 2, 2 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Byte:
					case fastgltf::ComponentType::UnsignedByte:
					case fastgltf::ComponentType::Short:
					case fastgltf::ComponentType::UnsignedShort:
					case fastgltf::ComponentType::Int:
					case fastgltf::ComponentType::UnsignedInt:
					case fastgltf::ComponentType::Double:
					case fastgltf::ComponentType::Invalid: break;
					}
					break;
				case fastgltf::AccessorType::Mat3:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::mat< float, 3, 3 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Byte:
					case fastgltf::ComponentType::UnsignedByte:
					case fastgltf::ComponentType::Short:
					case fastgltf::ComponentType::UnsignedShort:
					case fastgltf::ComponentType::Int:
					case fastgltf::ComponentType::UnsignedInt:
					case fastgltf::ComponentType::Double:
					case fastgltf::ComponentType::Invalid: break;
					}
					break;
				case fastgltf::AccessorType::Mat4:
					switch( accessor.componentType )
					{
					case fastgltf::ComponentType::Float:         fastgltf::copyFromAccessor< fastgltf::math::mat< float, 4, 4 > >( _asset, accessor, buffer.data() ); break;
					case fastgltf::ComponentType::Byte:
					case fastgltf::ComponentType::UnsignedByte:
					case fastgltf::ComponentType::Short:
					case fastgltf::ComponentType::UnsignedShort:
					case fastgltf::ComponentType::Int:
					case fastgltf::ComponentType::UnsignedInt:
					case fastgltf::ComponentType::Double:
					case fastgltf::ComponentType::Invalid: break;
					}
				break;
				case fastgltf::AccessorType::Invalid: break;
				}
			} // const auto& attribute : primitive.attributes
		} // auto& primitive : _mesh.primitives

		return mesh_asset;
	} // handleGltfMesh

	auto cAssetManager::handleGltfTexture( const fastgltf::Asset& _asset, fastgltf::Texture& _texture ) -> cShared_ptr<Asset_t>
	{
		if( !_texture.imageIndex.has_value() )
			return nullptr;

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

		if( target_buffer )
		{
			Assets::cTexture::eSourceType imageType = Assets::cTexture::eSourceType::kInvalid;
			switch( type )
			{
			case fastgltf::MimeType::PNG:  imageType = Assets::cTexture::eSourceType::kPNG; break;
			case fastgltf::MimeType::JPEG: imageType = Assets::cTexture::eSourceType::kJPG; break;
			}
			if( imageType == Assets::cTexture::eSourceType::kInvalid )
				return nullptr;

			return make_shared< Assets::cTexture >( image.name.c_str(), target_buffer, size, imageType );
		}

		return nullptr;
	}

	auto cAssetManager::loadPngFile( const std::filesystem::path& _path ) -> Assets::cAsset_List
	{
		// TODO: Standalone Png file loader.
		return {};
	} // loadPNGFile

	void cAssetManager::loadEmbedded( void )
	{
		constexpr auto resolution = cVector2u( 16 );
		constexpr auto byte_size  = static_cast< size_t >( resolution.x * resolution.y ) * 4ull; // x * y * RGBA
		std::vector< uint8_t > white_texture( byte_size, 0xff );

		Graphics::cRenderer::m_white_texture = createAsset< Assets::cTexture >( "White", white_texture.data(), resolution, Assets::cTexture::eFormat::kRGBA8, true );
	} // loadEmbedded


} // qw::
