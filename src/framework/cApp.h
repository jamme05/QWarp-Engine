#pragma once

#include <agc.h>
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

	qw::Assets::Shader::Shared_t   m_mesh_pair = nullptr;
	qw::Assets::Shader::Shared_t   m_post_pair = nullptr;

	qw::Scene::Shared_t m_scene = nullptr;

	std::vector< qw::Assets::Mesh::Shared_t > m_meshes = {};
};
