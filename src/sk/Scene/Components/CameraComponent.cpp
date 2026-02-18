/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "CameraComponent.h"

#include <sk/Scene/Managers/CameraManager.h>
#include <sk/Scene/Managers/SceneManager.h>
#include <sk/Window/Window.h>

namespace sk::Object::Components
{
	cCameraComponent::cCameraComponent( const Graphics::sViewport& _viewport, const Graphics::sScissor& _scissors, const sCameraSettings& _settings, const eType _type )
	: m_camera_settings( _settings )
	, m_viewport( _viewport )
	, m_scissor( _scissors )
	, m_type( _type )
	{
		calculateProjectionMatrix();
		Scene::cCameraManager::get().registerCamera( get_shared().Cast< cCameraComponent >() );
	} // cCamera

	cCameraComponent::cCameraComponent( cSerializedObject& _object )
	: cComponent( _object.GetBase< iComponent >().value() )
	{
		_object.BeginRead( this );
		m_type    = _object.ReadValue< eType >( "type" ).value_or( eType::kPerspective );
		m_layers_ = _object.ReadValue< uint64_t >( "layers" ).value_or( 1 );
		m_camera_settings.auto_resize = _object.ReadValue< bool >( "auto_resize" ).value_or( true );
		m_camera_settings.fov         = _object.ReadValue< float >( "fov" ).value_or( 70.0f );
		m_camera_settings.near   = _object.ReadValue< float >( "near" ).value_or( 0.01f );
		m_camera_settings.far    = _object.ReadValue< float >( "far" ).value_or( 2000.0f );
		if( m_camera_settings.auto_resize )
			_autoUpdate();
		else
		{
			m_camera_settings.aspect = _object.ReadValue< float >( "aspect" ).value_or( 0.5f );
			const auto viewport_pos          = _object.ReadValue< cVector2i32 >( "viewport_pos" ).value_or( cVector2i64{} );
			const auto viewport_size = _object.ReadValue< cVector2u64 >( "viewport_size" ).value_or( cVector2u64{} );
			const auto scissor_pos           = _object.ReadValue< cVector2i32 >( "scissor_pos" ).value_or( cVector2i64{} );
			const auto scissor_size  = _object.ReadValue< cVector2u64 >( "scissor_size" ).value_or( cVector2u64{} );

			m_viewport = {
				.x = viewport_pos.x,
				.y = viewport_pos.y,
				.width  = viewport_size.x,
				.height = viewport_size.y
			};

			m_scissor = {
				.x = scissor_pos.x,
				.y = scissor_pos.y,
				.width  = scissor_size.x,
				.height = scissor_size.y
			};
		}
		_object.EndRead();

		calculateProjectionMatrix();
		Scene::cCameraManager::get().registerCamera( get_shared().Cast< cCameraComponent >() );
	}

	cCameraComponent::~cCameraComponent()
	{
		if( const auto camera_manager = Scene::cCameraManager::getPtr() )
			camera_manager->UnregisterCamera( get_shared().Cast< cCameraComponent >() );
	}

	void cCameraComponent::renderTo( Graphics::Rendering::cRender_Context& _context )
	{
	} // begin

	void cCameraComponent::renderTo( Graphics::Rendering::cRender_Target& _context )
	{
		// TODO: Default pipeline for single render targets.
		// renderTo( default_render_target_context )
	} // renderTo

	void cCameraComponent::enabled( void )
	{
		Scene::cCameraManager::get().setCameraEnabled( get_shared().Cast< cCameraComponent >(), true );
	} // enabled

	void cCameraComponent::disabled( void )
	{
		Scene::cCameraManager::get().setCameraEnabled( get_shared().Cast< cCameraComponent >(), false );
	} // disabled

	void cCameraComponent::update( void )
	{
		bool refresh = false;
		if( Platform::GetMainWindow()->WasResizedThisFrame() )
		{
			_autoUpdate();
			refresh = true;
		}

		if( refresh || m_transform_->IsDirty() )
		{
			m_transform_->Update();
			m_transform_->GetWorld().inversed( m_view_proj_inv );
			m_view_proj_inv *= m_projection;
		}
	} // update

	void cCameraComponent::SetSettings( const sCameraSettings& _settings )
	{
		m_camera_settings = _settings;
		calculateProjectionMatrix();
	}

	auto cCameraComponent::Serialize() -> cSerializedObject
	{
		cSerializedObject object( this );
		object.AddBase( cComponent::Serialize() );
		object.WriteValue( "type",          static_cast< uint64_t >( m_type ) );
		object.WriteValue( "layers",        m_layers_ );
		object.WriteValue( "auto_resize",   m_camera_settings.auto_resize );
		object.WriteValue( "fov",           m_camera_settings.fov );
		object.WriteValue( "aspect",        m_camera_settings.aspect );
		object.WriteValue( "near_clip",     m_camera_settings.near );
		object.WriteValue( "far_clip",      m_camera_settings.far );
		object.WriteValue( "viewport_pos",  cVector2i64{ m_viewport.x, m_viewport.y } );
		object.WriteValue( "viewport_size", cVector2u64{ m_viewport.width, m_viewport.height } );
		object.WriteValue( "scissor_pos",   cVector2i64{ m_scissor.x, m_scissor.y } );
		object.WriteValue( "scissor_size",  cVector2u64{ m_scissor.width, m_scissor.height } );
		object.EndWrite();
		return object;
	}

	void cCameraComponent::_autoUpdate()
	{
		const auto& main_window  = *Platform::GetMainWindow();
		m_camera_settings.aspect = main_window.GetAspectRatio();
		const auto res = main_window.GetResolution();

		m_viewport = {
			.x = 0,
			.y = 0,
			.width  = res.x,
			.height = res.y
		};

		m_scissor = {
			.x = 0,
			.y = 0,
			.width  = res.x,
			.height = res.y
		};

		calculateProjectionMatrix();
	}

	void cCameraComponent::calculateProjectionMatrix( void )
	{
		m_projection = Math::Matrix4x4::AspectPerspective( m_camera_settings.aspect, m_camera_settings.fov, m_camera_settings.near, m_camera_settings.far );
	} // calculateProjectionMatrix
} // sk::Object::Components::

