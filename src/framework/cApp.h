#pragma once

#include <Misc/cSingleton.h>

#include "Assets/cMesh.h"
#include "Assets/cShader.h"
#include "Scene/cScene.h"
#include "Input/cInput.h"

namespace qw::Assets {
	class cShader;
}

namespace qw::Graphics::Agc
{
	class cWindow_context;
} // qw::Graphics::Agc

namespace qw::Graphics
{
	class cRender_context;
} // qw::Graphics::

class cApp : public qw::cSingleton< cApp >, qw::Input::iListener
{
	bool m_running = true;

public:

	 cApp( void );
	~cApp( void );

	bool running   ( void ) const         { return m_running; }
	void setRunning( const bool _running ){ m_running = _running; }

	bool onInput( const qw::Input::eType _type, const qw::Input::sEvent& _event ) override;

	void create ( void );
	void destroy( void );
	void run    ( void );

	static void custom_event( void );

	qw::cShared_ptr< qw::Assets::cShader > m_mesh_pair = nullptr;
	qw::cShared_ptr< qw::Assets::cShader > m_post_pair = nullptr;

	qw::cShared_ptr< qw::cScene > m_scene = nullptr;
	std::vector< qw::cShared_ptr< qw::Assets::cMesh > > m_meshes = {};
};
