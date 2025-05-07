#include "cApp.h"

#include "Assets/cMaterial.h"
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
#include "Scene/Components/cMeshComponent.h"
#include "Scene/Managers/cSceneManager.h"
#include "Scene/Objects/cCameraFlight.h"

#include "Graphics/cDepth_target.h"
#include "Graphics/cRender_target.h"
#include "Graphics/cUniforms.h"
#include "Graphics/cWindow_context.h"

#include "Containers/allocator.h"
#include "Misc/string_manipulation.h"

#include "Runtime/cRuntimeClass.h"
#include "Runtime/cRuntimeStruct.h"

cApp::cApp( void )
: iListener( qw::Input::eType::kAll, 10, true )
{
	qw::Input::setLogInputs( false );

} // cApp

cApp::~cApp( void )
{
	m_scene = nullptr;
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

class test1;
static constexpr auto target_test1 = qw::cRuntimeClass< test1 >( "test1", __FILE__, __LINE__ );
class test1 : public qw::get_inherits_t<>
{
	CREATE_CLASS_IDENTIFIERS( target_test1 )
};

class test2;
static constexpr auto target_test2 = qw::cRuntimeClass< test2, test1 >( "test2", __FILE__, __LINE__ );
class test2 : public qw::get_inherits_t< test1 >
{
	CREATE_CLASS_IDENTIFIERS( target_test2 )
};

class test3;
static constexpr auto target_test3 = qw::cRuntimeClass< test3 >( "test3", __FILE__, __LINE__ );
class test3 : public qw::get_inherits_t<>
{
	CREATE_CLASS_IDENTIFIERS( target_test3 )
};

GENERATE_CLASS( test4, test3 )
{
	CREATE_CLASS_BODY( test4 )
};

GENERATE_ALL_CLASS( test5, test4 )
	
};

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

	//auto list   = qw::cAssetManager::get().loadFolder( "data/" );
	auto list_1 = qw::cAssetManager::get().loadFile( "data/humanforscale.glb" );
	auto list_2 = qw::cAssetManager::get().loadFile( "data/heheToiletwithtextures.glb" );

	//qw::cAssetManager::get().loadFile( "data/mushroom.glb" );

	//auto cube          = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 0 );
	auto christopher_t = list_1.Get_Asset_Of_Type< qw::Assets::cTexture >();
	auto christopher_m = list_1.Get_Asset_Of_Type< qw::Assets::cMesh    >();
	auto toilet_t      = list_2.Get_Asset_Of_Type< qw::Assets::cTexture >();
	auto toilet_m      = list_2.Get_Asset_Of_Type< qw::Assets::cMesh    >();
	//auto mushroom      = qw::cAssetManager::get().getAssetAs< qw::Assets::cMesh >( 5 );

	m_scene = qw::cScene::create_shared( "Main" );
	m_scene->create_object< qw::Object::cCameraFlight >( "Camera Free Flight" )->setAsMain();
	//auto mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 1" );
	//mesh->getTransform().getPosition() = { -10.0f, 0.0f, 0.0f };
	//mesh->getTransform().update();
	//mesh->addComponent< qw::Object::Components::cMesh >( cube );

	auto mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 2" );
	mesh->getTransform().getPosition() = { -2.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMeshComponent >( christopher_m )->setTexture( const_cast< qw::Assets::Texture::ptr_t& >( christopher_t ) );

	mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 3" );
	mesh->getTransform().getPosition() = { 2.0f, 0.0f, 0.0f };
	mesh->getTransform().update();
	mesh->addComponent< qw::Object::Components::cMeshComponent >( toilet_m )->setTexture( toilet_t );

	//mesh = m_scene->create_object< qw::Object::iObject >( "Mesh Test 4" );
	//mesh->getTransform().getPosition() = { 10.0f, 0.0f, 0.0f };
	//mesh->getTransform().update();
	//mesh->addComponent< qw::Object::Components::cMesh >( cube )->setTexture( toilet_t );

	qw::cSceneManager::get().registerScene( m_scene );

	REGISTER_LISTENER( "Custom Event", &cApp::custom_event )

	qw::Assets::cMaterial material{ "Test", *m_mesh_pair };

	constexpr int arr1[]{ 1, 2, 3 };
	constexpr qw::array t1{ arr1 };
	constexpr qw::array t2{ 3, 2, 1 };
	constexpr qw::array t3{ t2 };
	//constexpr int* arr2 = t3;
	constexpr auto t4 = t2 + t1;
	constexpr auto v = qw::arr::concat< t1, { 1 } >::kValue;

	constexpr auto v2 = qw::args_hash< void >::kTypes;
	constexpr auto v3 = qw::validate_args( v2, true );

	print_types();
} // _create

void cApp::print_types( void )
{
	for( const auto& val : qw::type_map | std::views::values )
	{
		switch( val->type )
		{
		case qw::sType_Info::eType::kStandard:
		{
			printf( "Type: %s Name: %s Size: %lu \n", val->raw_name, val->name, static_cast< uint64_t >( val->size ) );
		}
		break;
		case qw::sType_Info::eType::kStruct:
		{
			printf( "Struct: %s Name: %s Size: %lu \n", val->raw_name, val->name, static_cast< uint64_t >( val->size ) );
			for( auto struct_info = static_cast< const qw::sStruct_Type_Info* >( val ); const auto& member : struct_info->members | std::views::values )
			{
				if( auto member_type = member.get_type() )
				{
					printf( "   Type: %s Name: %s Size: %lu Offset: %lu \n", member_type->raw_name, member.display_name, static_cast< uint64_t >( member.size ), static_cast< uint64_t >( member.offset ) );
				}
			}
		}
		break;
		}
	}
}

void cApp::custom_event( void )
{
	printf( "Custom Event go brrr\n" );
} // custom_event

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

	// Removed due to NDA

	qw::cSceneManager::render();

	//auto& r_ctx = m_render_context->getBackContext();

	static uint64_t counter = 0;
	if( counter++ % 100 == 0 )
	{
		POST_EVENT( "Custom Event" )
	}

	auto& window_context = qw::Graphics::cRenderer::get().getWindowContext();

	window_context.begin( viewport, scissor );
	window_context.clear( qw::Graphics::eClear::kAll, qw::Color::kBlack );

	auto& ctx = window_context.getBackContext();

	// Removed due to NDA

	window_context.end();

} // run
