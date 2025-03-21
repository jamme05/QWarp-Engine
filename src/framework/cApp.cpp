#include "cApp.h"

#include "Assets/cMesh.h"
#include "Assets/cShader.h"
#include "Assets/iAsset.h"
#include "Assets/cTexture.h"
#include "Assets/Manager/cAsset_manager.h"

#include "Embedded/Shaders.h"

#include "Graphics/cRenderer.h"
#include "Graphics/cRender_context.h"

#include "Math/Types.h"
#include "Memory/Tracker/cTracker.h"
#include "Platform/cPlatform.h"

#include "Scene/cScene.h"
#include "Scene/Components/cMesh.h"
#include "Scene/Managers/cSceneManager.h"
#include "Scene/Objects/cCameraFlight.h"

#include "Graphics/cDepth_target.h"
#include "Graphics/cRender_target.h"
#include "Graphics/cWindow_context.h"

cApp::cApp( void )
: iListener( qw::Input::eType::kAll, 10, true )
{
	//qw::Input::setLogInputs( true );

} // cApp

cApp::~cApp( void )
{

} // ~cApp

bool cApp::onInput( const qw::Input::eType _type, const qw::Input::sEvent& _event )
{
	switch( _type )
	{
	case qw::Input::kButton_Down:
	{
		if( _event.pad->button == qw::Input::eButton::kOptions )
			m_running = false;
	}
	break; // qw::Input::kButton_Down

	default:
		break;
	}

	return false;
} // onInput

void cApp::create( void )
{
	qw::cPlatform::initialize();
	qw::Graphics::cRenderer::initialize();
	qw::cAssetManager::initialize();
	qw::cSceneManager::initialize();

	qw::Graphics::cRenderer::get().registerVertexAttribute( qw::Graphics::sBaseVertex::g_attribute_regs );

	m_mesh_pair      = qw::Assets::cShader::create_shared( "Mesh Vertex", Embedded::Shaders::Deferred::Mesh::Vertex, ePrimType_t::kTriList );
	auto temp_shader = qw::Assets::cShader::create_shared( "Mesh Pixel",  Embedded::Shaders::Deferred::Mesh::Pixel,  ePrimType_t::kTriList );

	m_mesh_pair->setAttributes( qw::Graphics::sBaseVertex::g_attribute_regs );
	m_mesh_pair->linkShader( temp_shader );

	m_post_pair = qw::Assets::cShader::create_shared( "Post Vertex", Embedded::Shaders::Post_processing::Vertex, ePrimType_t::kRectList );
	temp_shader = qw::Assets::cShader::create_shared( "Post Pixel",  Embedded::Shaders::Post_processing::Pixel,  ePrimType_t::kRectList );

	m_post_pair->linkShader( temp_shader );

	qw::cAssetManager::get().loadFile( "data/cube.glb" );
	qw::cAssetManager::get().loadFile( "data/humanforscale.glb" );
	qw::cAssetManager::get().loadFile( "data/heheToiletwithtextures.glb" );
	qw::cAssetManager::get().loadFile( "data/mushroom.glb" );

	auto cube          = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 0 );
	auto christopher_t = qw::cAssetManager::get().getAssetAs< qw::Assets::cTexture >( 1 );
	auto christopher_m = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 2 );
	auto toilet_t      = qw::cAssetManager::get().getAssetAs< qw::Assets::cTexture >( 3 );
	auto toilet_m      = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 4 );
	auto mushroom      = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 5 );

	m_scene = qw::cScene::create_shared( "Main" );
	m_scene->create_object< qw::Object::cCameraFlight >( "Camera Free Flight" )->setAsMain();
	auto mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 1" );
	mesh->getTransform().getPosition() = { -10.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMesh >( cube );

	mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 2" );
	mesh->getTransform().getPosition() = { -2.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMesh >( christopher_m )->setTexture( christopher_t );

	mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 3" );
	mesh->getTransform().getPosition() = { 2.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMesh >( toilet_m )->setTexture( toilet_t );

	mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 4" );
	mesh->getTransform().getPosition() = { 10.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMesh >( mushroom )->setTexture( toilet_t );

	qw::cSceneManager::get().registerScene( m_scene );

} // _create

void cApp::destroy( void )
{
	qw::cSceneManager::deinitialize();
	qw::cAssetManager::deinitialize();
	qw::Graphics::cRenderer::deinitialize();
	qw::cPlatform::deinitialize();

} // _destroy

void cApp::run( void )
{
	qw::cPlatform::Update();
	qw::cSceneManager::get().update();

	auto& resolution   = qw::Graphics::cRenderer::get().getWindowResolution();
	qw::cVector2i ires = resolution;

	qw::Graphics::sViewport viewport{ 0, 0, resolution.x, resolution.y };
	qw::Graphics::sScissor  scissor { 0, 0, ires.x, ires.y };

	if( !qw::Graphics::cRenderer::get().getRenderContext().setShader( *m_mesh_pair ) )
		__debugbreak();

	qw::cSceneManager::render();

	// Post-processing removed due to NDA

} // run
