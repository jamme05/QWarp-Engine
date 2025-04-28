/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cCameraComponent.h"

#include "Scene/Managers/cCameraManager.h"
#include "Scene/Managers/cSceneManager.h"

namespace qw::Object::Components
{
	cCameraComponent::cCameraComponent( const Graphics::sViewport& _viewport, const Graphics::sScissor& _scissors, const sCameraSettings& _settings, const eType _type )
	: m_camera_settings( _settings )
	, m_viewport( _viewport )
	, m_scissor( _scissors )
	, m_type( _type )
	{
		calculateProjectionMatrix();
		Scene::cCameraManager::get().registerCamera( get_shared_this() );
	} // cCamera

	void cCameraComponent::renderTo( Graphics::cRender_context& _context )
	{
		cSceneManager::get().render_with( *this, _context );
	} // begin

	void cCameraComponent::renderTo( Graphics::cRender_target& _context )
	{
		// TODO: Default pipeline for single render targets.
		// renderTo( default_render_target_context )
	} // renderTo

	void cCameraComponent::enabled( void )
	{
		Scene::cCameraManager::get().setCameraEnabled( get_shared_this(), true );
	} // enabled

	void cCameraComponent::disabled( void )
	{
		Scene::cCameraManager::get().setCameraEnabled( get_shared_this(), false );
	} // disabled
	void cCameraComponent::update( void )
	{
		m_transform.update();
		m_transform.getWorld().inversed_fast( m_view_proj_inv );
		m_view_proj_inv = m_view_proj_inv * m_projection;
	} // update

	void cCameraComponent::calculateProjectionMatrix( void )
	{
		const bool vertical = m_camera_settings.aspect > 1.0f;

		// TODO: Check if there's a less "tengine" way of doing this.
		const auto cotan = 1.0f / Math::tan( Math::degToRad( m_camera_settings.fov / 2.0f ) );
		const auto depth = m_camera_settings.far - m_camera_settings.near;
		const auto xx    = vertical ? ( -cotan / m_camera_settings.aspect ) : ( -cotan );
		const auto yy    = vertical ? ( cotan ) : ( cotan * m_camera_settings.aspect );

		m_projection = {
			cVector4f( xx, 0.0f, 0.0f, 0.0f ),
			cVector4f( 0.0f, yy, 0.0f, 0.0f ),
			cVector4f( 0.0f, 0.0f, ( m_camera_settings.near + m_camera_settings.far ) / depth, 1.0f ),
			cVector4f( 0.0f, 0.0f, -2.0f * m_camera_settings.far * m_camera_settings.near / depth, 0.0f ),
		};
	} // calculateProjectionMatrix
} // qw::Object::Components::
