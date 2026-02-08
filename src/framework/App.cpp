

#include "App.h"

#include <sk/Assets/Asset.h>
#include <sk/Assets/Texture.h>
#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Assets/Utils/Asset_List.h>
#include <sk/Graphics/Renderer.h>
#include <sk/Graphics/Pipelines/Deferred_Pipeline.h>
#include <sk/Graphics/Pipelines/Pipeline.h>
#include <sk/Graphics/Rendering/Render_Context.h>
#include <sk/Graphics/Utils/RenderUtils.h>
#include <sk/Input/Keyboard.h>
#include <sk/Input/Mouse.h>
#include <sk/Math/Types.h>
#include <sk/Memory/Tracker/Tracker.h>
#include <sk/Misc/UUID.h>
#include <sk/Platform/Time.h>
#include <sk/Platform/ImGui/ImGuiHelper.h>
#include <sk/Platform/Window/Window_Base.h>
#include <sk/Reflection/RuntimeClass.h>
#include <sk/Reflection/RuntimeStruct.h>
#include <sk/Scene/Scene.h>
#include <sk/Scene/Components/LightComponent.h>
#include <sk/Scene/Components/MeshComponent.h>
#include <sk/Scene/Components/SpinComponent.h>
#include <sk/Scene/Managers/SceneManager.h>
#include <sk/Scene/Objects/CameraFlight.h>

#include <imgui.h>

#include <print>
#include <random>
#include <vector>



cApp* cApp::m_running_instance_ = nullptr;

cApp::cApp()
: iListener( sk::Input::eInputType::kKey | sk::Input::kMouse_Down, 10, true )
{
	m_running_instance_ = this;
	sk::Input::setLogInputs( false );
	
	sk::cAsset_Manager::init();

	m_main_window_ = sk::Platform::CreateWindow( "Main Window", { 1280, 720 } );

	sk::Graphics::InitRenderer();
	sk::Graphics::Utils::InitUtils();
	m_main_window_->Init();
	
	m_windows.emplace( m_main_window_ );

	SK_ERR_IFN( m_main_window_->SetVisibility( true ),
		"Unable to show window." )


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	sk::Gui::InitImGui( m_main_window_ );

	sk::cSceneManager::init();
} // cApp

cApp::~cApp()
{
	destroy();
	m_scene = nullptr;
} // ~cApp

sk::Input::response_t cApp::onInput( const uint32_t _type, const sk::Input::sEvent& _event )
{
	static bool captured = false;
	if( _type == sk::Input::kMouse_Down && _event.mouse->button == sk::Input::Mouse::kLeft )
	{
		m_main_window_->SetMouseCapture( true );
		captured = true;
	}
	if( _type == sk::Input::kKey_Down && _event.keyboard->key == sk::Input::Keyboard::kEscape )
	{
		if( captured )
		{
			m_main_window_->SetMouseCapture( false );
			captured = false;
		}
		else
			return sk::Input::eResponse::kQuit;
	}

	return true;
} // onInput

void cApp::create()
{
	auto& asset_m = sk::cAsset_Manager::get();

	auto list_1 = asset_m.loadFile( "models/humanforscale.glb" );
	auto list_2 = asset_m.loadFile( "models/heheToiletwithtextures.glb" );
	const auto shader_frag = *asset_m.loadFile( "shaders/gpass.frag" ).begin();
	const auto shader_vert = *asset_m.loadFile( "shaders/default.vert" ).begin();
	asset_m.loadFile( "shaders/screen.vert" );
	asset_m.loadFile( "shaders/deferred.frag" );
	
	// TODO: Create a material instance class.
	
	// sk::Graphics::cRenderer::get().SetPipeline( SK_SINGLE( sk::Graphics::cForward_Pipeline, m_main_window_ ) );
	sk::Graphics::cRenderer::get().SetPipeline( SK_SINGLE( sk::Graphics::cDeferred_Pipeline, m_main_window_ ) );

	const auto christopher_t = list_1.GetAssetOfType< sk::Assets::cTexture >();
	auto christopher_m       = list_1.GetAssetOfType< sk::Assets::cMesh    >();
	const auto toilet_t      = list_2.GetAssetOfType< sk::Assets::cTexture >();
	auto toilet_m            = list_2.GetAssetOfType< sk::Assets::cMesh    >();

	auto [ scene_meta, _ ] = asset_m.CreateAsset< sk::cScene >( "Main Scene", "scenes/main_scene.skscene" );
	m_scene = scene_meta;
	m_scene->create_object< sk::Object::cCameraFlight >( "Camera Free Flight" )->setAsMain();

	auto mat1 = asset_m.CreateAsset< sk::Assets::cMaterial >( "Material Test", "materials/mat1.skmat1",
		sk::Graphics::Utils::cShader_Link{ shader_vert, shader_frag } );
	mat1.second->SetTexture( "mainTexture", christopher_t );

	auto mesh = m_scene->create_object< sk::Object::cObject >( "Mesh Test 2" );
	mesh->GetTransform().SetLocalPosition( { 0.0f, -16.0f, 90.0f } );
	auto component = mesh->AddComponent< sk::Object::Components::cMeshComponent >( christopher_m, mat1.first );
	component->enabled();
	component->SetLocalScale( sk::cVector3f{ 100.0f } );
	
		
	auto mat2 = asset_m.CreateAsset< sk::Assets::cMaterial >( "Material Test 2", "materials/mat1.skmat2",
		sk::Graphics::Utils::cShader_Link{ shader_vert, shader_frag } );
	mat2.second->SetTexture( "mainTexture", toilet_t );

	mesh = m_scene->create_object< sk::Object::cObject >( "Mesh Test 3" );
	mesh->GetTransform().SetLocalPosition( { -0.1f, 0.0f, 2.5f } );
	auto spin_component = mesh->AddComponent< sk::Object::Components::cSpinComponent >( sk::cVector3f{ 0.0f, 5.0f, 0.0f } );
	component = mesh->AddComponent< sk::Object::Components::cMeshComponent >( toilet_m, mat2.first );
	component->enabled();
	component->SetParent( spin_component );
	
	sk::Scene::Light::sSettings light_settings{};
	auto light_object = m_scene->create_object< sk::Object::cObject >( "Directional Light" );
	light_settings.shadow_resolution = 1024;
	light_settings.casts_shadows     = true;
	auto directional_light = light_object->AddComponent< sk::Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );

	light_object = m_scene->create_object< sk::Object::cObject >( "Point Light" );
	light_settings.type              = sk::Scene::Light::eType::kPoint;
	light_settings.casts_shadows     = false;
	light_settings.shadow_resolution = 256;
	auto point_light = light_object->AddComponent< sk::Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );

	light_object = m_scene->create_object< sk::Object::cObject >( "Spot Light 1" );
	light_settings.type              = sk::Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	auto spot_light = light_object->AddComponent< sk::Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );
	light_object->GetTransform().SetLocalPosition( { 0.0f, 5.0f, 0.0f } );

	light_object = m_scene->create_object< sk::Object::cObject >( "Spot Light 2" );
	light_settings.type              = sk::Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	spot_light = light_object->AddComponent< sk::Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 90.0f, 0.0f } );
	light_object->GetTransform().SetLocalPosition( { 0.0f, 5.0f, 0.0f } );

	light_object = m_scene->create_object< sk::Object::cObject >( "Spot Light 3" );
	light_settings.type              = sk::Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	spot_light = light_object->AddComponent< sk::Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 180.0f, 0.0f } );
	light_object->GetTransform().SetLocalPosition( { 0.0f, 5.0f, 0.0f } );

	// Fuck it we ball
	constexpr auto kGridWidth = 5;
	std::random_device rd;
	std::mt19937 gen( rd() );
	std::uniform_real_distribution dis( -25.0f, 25.0f );
	for( int64_t x = -kGridWidth; x < kGridWidth; ++x )
	{
		for( int64_t y = -kGridWidth; y < kGridWidth; ++y )
		{
			auto mesh_object = m_scene->create_object< sk::Object::cObject >( "Clone" );
			mesh_object->GetTransform().SetLocalPosition( { x * 2, 0.0f, y * 2 } );
			auto spin = mesh_object->AddComponent< sk::Object::Components::cSpinComponent >( sk::cVector3f{ 0.0f, dis( gen ), 0.0f } );
			auto mesh_component = mesh_object->AddComponent< sk::Object::Components::cMeshComponent >( christopher_m, mat1.first );
			mesh_component->enabled();
			mesh_component->SetLocalRotation( { -90.0f, 0.0f, 0.0f } );
			mesh_component->SetParent( spin );
		}
	}

	sk::cSceneManager::get().RegisterScene( scene_meta );
} // create

void cApp::print_types()
{
	std::vector< sk::cUUID > uuids{ 1024 };
	for( auto& uuid : uuids )
		uuid = sk::GenerateRandomUUID();

	for( auto& uuid : uuids )
		std::println( "{} {}", uuid.ToString(), uuid.ToString( false ) );

	auto& types = sk::Reflection::cType_Manager::get().GetTypes();

	for( const auto& val : types | std::views::values )
	{
		std::println();
		switch( val->type )
		{
		case sk::sType_Info::eType::kStandard:
		{
			std::println( "Type: {} Name: {} Size: {}", val->raw_name, val->name, static_cast< uint64_t >( val->size ) );
		}
		break;
		case sk::sType_Info::eType::kStruct:
		{
			std::println( "Struct: {} Name: {} Size: {}", val->raw_name, val->name, static_cast< uint64_t >( val->size ) );
			for( const auto struct_info = val->as_struct_info(); const auto& member : struct_info->members | std::views::values )
			{
				const auto member_type = member.type;
				std::println( "   Type: {} Name: {} Size: {} Offset: {}", member_type->raw_name, member.display_name, static_cast< uint64_t >( member.size ), static_cast< uint64_t >( member.offset ) );
			}
		}
		break;
		case sk::sType_Info::eType::kClass:
		{
			const auto class_info = val->as_class_info();
			auto variables = class_info->runtime_class->getVariables();
			auto functions = class_info->runtime_class->getFunctions();
			std::println( "Class: {}, Name: {}, Size: {}, Variables: {}, Functions: {}",
				class_info->raw_name, class_info->name, class_info->size,
				variables.size(), functions.size() );

			std::println( "Variables:" );
			for( const auto variable : variables | std::views::values )
			{
				auto type        = variable->getType()->raw_name;
				auto name        = variable->getName();
				auto visibility  = variable->getVisibilityStr();
				auto is_static   = variable->getIsStatic  () ? "static " : "";
				auto is_const    = variable->getIsReadOnly() ? "const "  : "";
				auto flags       = variable->getFlags();
				// Example: protected int hello
				std::println( "    {}: {}{}{} {} Flags: {}", visibility, is_static, is_const, type, name, flags );
			}

			std::println( "Functions:" );
			for( const auto function : functions | std::views::values )
			{
				auto name        = function->getName();
				auto return_type = function->getReturnType()->raw_name;
				auto args_types  = function->getArgumentTypes()->to_raw_string();
				auto visibility  = function->getVisibilityStr();
				auto flags       = function->getFlags();
				auto is_static   = function->getIsStatic() ? " static" : "";
				auto is_const    = function->getIsReadOnly() ? "const " : "";
				// Example: static private void test_function( bool, int )
				std::println( "    {}:{} {} {}( {} ) {} Flags: {}",
					visibility, is_static, return_type, name, args_types, is_const, flags );
			}
		}
		break;
		default: break;
		}
	}
}

void cApp::destroy()
{
	m_scene = nullptr;
	
	sk::cSceneManager::shutdown();

	sk::Gui::ImGuiShutdown();
	ImGui::DestroyContext();

	for( const auto& window : m_windows )
		SK_DELETE( window );

	sk::cAsset_Manager::shutdown();

	// I don't really like the choice of having the renderer shut down after the asset system,
	// with the asset system being started before the renderer.
	// TODO: Add some RegisterRendererListeners function or something in the future.
	sk::Graphics::Utils::ShutdownUtils();
	sk::Graphics::cRenderer::shutdown();
	sk::cStringIDManager::shutdown();

} // destroy

void cApp::run()
{
	sk::Gui::ImGuiNewFrame();
	ImGui::ShowDemoWindow();

	sk::Time::Update();
	
	sk::cSceneManager::get().update();
	sk::cEventManager::get().postEvent( sk::Object::kUpdate );
	auto& pipeline = *sk::Graphics::cRenderer::get().GetPipeline();
	
	// TODO: Have the input manager push a mouse relative event with the relative being at 0, 0 every frame
	
	sk::Graphics::cRenderer::get().Update();
	
	pipeline.Execute();
	sk::Gui::ImGuiRender();
	m_main_window_->SwapBuffers();

} // run
