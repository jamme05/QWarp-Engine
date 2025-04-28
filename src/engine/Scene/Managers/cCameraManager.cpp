/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cCameraManager.h"

#include "cSceneManager.h"
#include "Graphics/cRenderer.h"
#include "Scene/Components/cCameraComponent.h"

namespace qw::Scene
{
	cCameraManager::cCameraManager( void )
	{
	} // cCameraManager

	void cCameraManager::registerCamera( const cShared_ptr< camera_t >& _camera )
	{
		_camera->m_camera_id = m_cameras.size();

		m_cameras[ _camera->m_camera_id ] = _camera;
	} // registerCamera

	void cCameraManager::setMainCamera( const cShared_ptr< camera_t >& _camera )
	{
		// Add previous main camera if it was enabled.
		if( m_main_camera.m_enabled )
			m_cameras[ m_main_camera.m_camera->m_camera_id ] = m_main_camera.m_camera;

		if( _camera->m_camera_id == INVALID_CAMERA_ID )
		{
			printf( "WARNING: Camera isn't registered. You won't be stopped for doing this. But it isn't recommended." );
			m_main_camera.m_enabled = false;
		}
		else
		{
			// If an element was erased it means the camera was enabled.
			m_main_camera.m_enabled = ( m_enabled_cameras.erase( _camera->m_camera_id ) == 1 );
		}

		m_main_camera.m_camera = _camera;
	} // setMainCamera

	void cCameraManager::setCameraEnabled( const cShared_ptr< camera_t >& _camera, const bool _enable )
	{
		if( _camera->m_camera_id == INVALID_CAMERA_ID )
		{
			printf( "WARNING: Camera isn't registered. Register it before you add it to the list." );
			return;
		}

		if( _enable )
			m_enabled_cameras[ _camera->m_camera_id ] = _camera;
		else
			m_enabled_cameras.erase( _camera->m_camera_id );

	} // setCameraEnabled

	void cCameraManager::render( void )
	{
		for( auto& camera : m_enabled_cameras )
		{
			camera.second->renderAuto();
		}

		m_main_camera.m_camera->update(); // TODO: Get rid of force update.
		cSceneManager::get().render_with( *m_main_camera.m_camera, Graphics::cRenderer::get().getRenderContext() );
	} // render
} // qw::Scene::