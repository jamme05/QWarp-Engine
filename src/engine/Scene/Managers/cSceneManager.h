/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "cCameraManager.h"

#include "Graphics/cRender_context.h"
#include "Misc/cSingleton.h"
#include "Misc/Smart_ptrs.h"

namespace qw
{
	class cScene;

	class cSceneManager : public cSingleton< cSceneManager >
	{

	public:
		struct sObjectBuffer
		{
			cMatrix4x4f view_proj_inv;
			cMatrix4x4f world;
		};
		// TODO: Logic?
		 cSceneManager( void );
		~cSceneManager( void );

		void registerScene( const cShared_ptr< cScene >& _scene );

		void update( void );

		static void render( void );

		void render_with( qw::Object::Components::cCameraComponent& _camera, Graphics::cRender_context& _context, const bool _swap = true );

		static auto get_active_context( void ){ return m_active_context; }

		static auto& get_buffer( void ){ return m_out_buffer; }

	private:

		static sObjectBuffer* m_out_buffer;

		static Graphics::cRender_context* m_active_context;

		vector< cShared_ptr< cScene > > m_scenes;
	};
} // qw::
