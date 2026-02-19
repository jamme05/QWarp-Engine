/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */


#include "SceneManager.h"

#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Assets/Utils/Asset_List.h>
#include <sk/Scene/Managers/CameraManager.h>
#include <sk/Scene/Managers/Internal_Component_Manager.h>
#include <sk/Scene/Managers/Layer_Manager.h>
#include <sk/Scene/Managers/Light_Manager.h>

using namespace sk;

namespace
{
	void loadSceneFile( const std::filesystem::path& _path, Assets::cAsset_List& _metas, const Assets::eAssetTask _load_task )
	{
		if( _load_task == Assets::eAssetTask::kLoadMeta )
		{
			_metas.AddAsset( sk::MakeShared< cAsset_Meta >( _path.filename().string(), kTypeInfo< cScene > ) );
			return;
		}

		if( _load_task == Assets::eAssetTask::kUnloadAsset )
			return;

		auto& meta = *_metas.begin();

		if( _load_task == Assets::eAssetTask::kSaveAsset )
		{
			if( !meta->IsLoaded() )
				return;

			meta->LockAsset();
			auto asset = meta->GetAsset();

			auto serialized_meta = asset->Serialize();
			const auto json = serialized_meta.CreateJSON();
			std::ofstream out_meta_file{ _path, std::ofstream::out | std::ofstream::binary };
			out_meta_file << json;
			out_meta_file.close();
			meta->UnlockAsset();

			return;
		}

		std::ifstream json_file{ _path, std::ifstream::in | std::ifstream::binary | std::ifstream::ate };
		json_file.seekg( 0, json_file.end );

		const size_t size = json_file.tellg();
		json_file.seekg( 0, json_file.beg );

		std::string json_str( size, 0 );
		json_file.read( json_str.data(), json_str.size() );

		simdjson::ondemand::parser parser;
		auto doc = parser.iterate( json_str );
		auto object = cSerializedObject{ doc.get_object() };

		meta->setAsset( SK_SINGLE( cScene, object ) );
	}
} // sk::

cSceneManager::cSceneManager()
{
	cAsset_Manager::get().AddFileLoaderForExtension( "skscene", loadSceneFile );

	cEventManager::init();
	Scene::cCameraManager::init();
	Scene::cLayer_Manager::init();
	Scene::cLight_Manager::init();
	Scene::cInternal_Component_Manager::init();
}
cSceneManager::~cSceneManager()
{
	m_scenes_.clear();
	m_loaded_scenes_.clear();

	Scene::cInternal_Component_Manager::shutdown();
	Scene::cLight_Manager::shutdown();
	Scene::cCameraManager::shutdown();
	Scene::cLayer_Manager::shutdown();
	cEventManager::shutdown();

	cAsset_Manager::get().RemoveFileLoaders( { "skscene" } );
} // ~cSceneManager

void cSceneManager::RegisterScene( const cShared_ptr< cAsset_Meta >& _scene_meta )
{
	if( m_scenes_.contains( _scene_meta->GetUUID() ) )
	{
		SK_WARNING( sk::Severity::kEngine, "Warning: Scene \"{}\" is already registered", _scene_meta->GetName().view() )
		return;
	}

	m_scenes_[ _scene_meta->GetUUID() ] = _scene_meta;
} // registerScene
void cSceneManager::UnregisterScene( const cUUID& _scene_uuid )
{
	// TODO: Warn if it wasn't found.
	if( m_scenes_.contains( _scene_uuid ) )
	{
		UnloadScene( _scene_uuid );
		m_scenes_.erase( _scene_uuid );
	}
}

void cSceneManager::LoadScene( const cUUID& _scene_uuid )
{
	if( m_loaded_scenes_.contains( _scene_uuid ) )
		return; // TODO: Throw warning

	auto& ref = m_loaded_scenes_[ _scene_uuid ] = cAsset_Ref< cScene >{};
	ref.on_changed += on_scene_loaded;
	// TODO: Warn if the scene isn't registered.
	ref = m_scenes_[ _scene_uuid ].Lock();
}

void cSceneManager::UnloadScene( const cUUID& _scene_uuid )
{
	m_loaded_scenes_.erase( _scene_uuid );
}

void cSceneManager::update()
{
	if( !m_recently_loaded_scenes_.empty() )
	{
		m_scene_mutex_.lock();
		const auto& scene = *m_recently_loaded_scenes_.back();
		m_recently_loaded_scenes_.pop();
		m_scene_mutex_.unlock();

		for( auto& object : scene.GetObjects() )
			object->registerRecursive();

		for( auto& object : scene.GetObjects() )
			object->enableRecursive();
	}
} // update

void cSceneManager::render()
{
	// TODO: Add logic to prepare for render.
	Scene::cCameraManager::get().render();
} // render

void cSceneManager::on_scene_loaded( Assets::eEventType _action, cAsset_Ref< cScene >& _ref )
{
	auto& self = get();
	self.m_scene_mutex_.lock();
	self.m_recently_loaded_scenes_.emplace( _ref.Get() );
	self.m_scene_mutex_.unlock();
}
