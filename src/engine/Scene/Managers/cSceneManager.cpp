/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */


#include "cSceneManager.h"

#include "Graphics/cRenderer.h"
#include "Scene/cScene.h"
#include "cCameraManager.h"
#include "cEventManager.h"
#include "Scene/Components/cCamera.h"

namespace qw
{
	Graphics::cRender_context*    cSceneManager::m_active_context = nullptr;
	cSceneManager::sObjectBuffer* cSceneManager::m_out_buffer = nullptr;
	sce::Agc::Core::Buffer*       cSceneManager::m_actual_buffer = nullptr;

	cSceneManager::cSceneManager( void )
	{
		Scene::cCameraManager::initialize();
		cEventManager::initialize();
		m_actual_buffer = static_cast< sce::Agc::Core::Buffer* >( Memory::cMemory_shared::alloc( sizeof( sce::Agc::Core::Buffer ), sce::Agc::Alignment::kBuffer ) );
	}
	cSceneManager::~cSceneManager( void )
	{
		m_scenes.clear();
		cEventManager::deinitialize();
		Scene::cCameraManager::deinitialize();
	} // ~cSceneManager

	void cSceneManager::registerScene( const cShared_ptr< cScene >& _scene )
	{
		m_scenes.push_back( _scene );
	} // registerScene

	void cSceneManager::update( void )
	{
		cEventManager::get().postEvent( Object::kUpdate );
	} // update

	void cSceneManager::render( void )
	{
		// TODO: Add logic to prepare for render.
		Scene::cCameraManager::get().render();
		m_active_context = nullptr;
	} // render

	void cSceneManager::render_with( Scene::camera_t& _camera, Graphics::cRender_context& _context, const bool _swap )
	{
		m_active_context = &_context;
		_context.begin( _camera.getViewport(), _camera.getScissor() );
		_context.clear( Graphics::eClear::kAll, Color::kBlack );

		initializeConstantBuffer( m_actual_buffer, m_out_buffer, sizeof( sObjectBuffer ) );

		// TODO: Use some spacial indexing for rendering.
		// Idea: Bounding cubes covering the entire world
		// The closer to the camera the more layers the bounding boxes can have.
		cEventManager::get().postEvent( Object::kRender );

		_context.end( _swap );
	} // render_with
} // qw::