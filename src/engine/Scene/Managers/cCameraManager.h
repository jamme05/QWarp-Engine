/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Misc/cSingleton.h"
#include "Misc/Smart_ptrs.h"
#include "Scene/cObject.h"

namespace qw::Graphics
{
	class cRender_context;
} // qw::Graphics

namespace qw::Object::Components
{
	class cCameraComponent;
} // qw::Object::Components

#define INVALID_CAMERA_ID uint64_t( -1 )

namespace qw::Scene
{
	typedef Object::Components::cCameraComponent camera_t;

	class cCameraManager : public cSingleton< cCameraManager >
	{

	struct sMainCamera
	{
		cShared_ptr< camera_t > m_camera  = nullptr;
		bool                    m_enabled = false;
	};

	public:
		cCameraManager( void );

		void registerCamera( const cShared_ptr< camera_t >& _camera );

		void setMainCamera   ( const cShared_ptr< camera_t >& _camera );
		auto getMainCamera   ( void ){ return m_main_camera.m_camera; }

		void setCameraEnabled( const cShared_ptr< camera_t >& _camera, const bool _enable );

		void render( void );

	private:

	// Main camera gets rendered to screen
		sMainCamera m_main_camera = {};

	// ALl registered cameras
		map< uint64_t, cShared_ptr< camera_t > > m_cameras;
	// All cameras being rendered to ( if they have a set render_context or render_target )
		map< uint64_t, cShared_ptr< camera_t > > m_enabled_cameras;
	};
} // qw::Scene
