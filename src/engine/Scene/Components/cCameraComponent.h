/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "iComponent.h"
#include "Graphics/cRender_context.h"
#include "Math/cMatrix4x4.h"
#include "Types/graphics_types.h"

namespace qw::Scene
{
	class cCameraManager;
} // qw::Scene

namespace qw::Object::Components
{
	QW_COMPONENT_CLASS( CameraComponent )
	{
	public:
		struct sCameraSettings
		{
			float fov;
			float aspect;
			float near;
			float far;
		};

		enum class eType : uint8_t
		{
			kPerspective,
			kOrthographic,
		};

		cCameraComponent( const Graphics::sViewport& _viewport, const Graphics::sScissor& _scissors, const sCameraSettings& _settings, const eType _type );

		void renderTo( Graphics::cRender_context& _context );
		void renderTo( Graphics::cRender_target&  _context );

		void enabled ( void ) override;
		void disabled( void ) override;

		void renderAuto( void ){ if( m_render_target ) renderTo( *m_render_target ); }

		void update( void ) override;

		auto& getViewport( void )       { return m_viewport; }
		auto& getViewport( void ) const { return m_viewport; }
		auto& getScissor ( void )       { return m_scissor;  }
		auto& getScissor ( void ) const { return m_scissor;  }

		auto& getViewProjInv( void ) const { return m_view_proj_inv; }
		auto& getProjection ( void ) const { return m_projection; }

	private:

		void calculateProjectionMatrix( void );

		sCameraSettings m_camera_settings;

		// TODO: Add support for render target, custom render pipeline in that case.
		Graphics::cRender_target* m_render_target;

		Graphics::sViewport m_viewport;
		Graphics::sScissor  m_scissor;

		// TODO: Rename to view_proj as view is the inverted world.
		cMatrix4x4f m_view_proj_inv;
		cMatrix4x4f m_projection;

		eType m_type;

		constexpr static auto invalid_camera_id = ~0llu;

		uint64_t m_camera_id = invalid_camera_id;

		friend class Scene::cCameraManager;
	};
} // qw::Object::Components
