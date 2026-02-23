//
// Created by jag on 2026-02-03.
//

#include "Editor.h"

#include <sk/Assets/Material.h>
#include <sk/Assets/Management/Asset_Manager.h>
#include <sk/Editor/Managers/SelectionManager.h>
#include <sk/Editor/Tabs/AssetViewTab.h>
#include <sk/Editor/Tabs/ObjectListTab.h>
#include <sk/Editor/Tabs/PlaceholderTab.h>
#include <sk/Editor/Tabs/SceneViewportTab.h>
#include <sk/Graphics/Renderer.h>
#include <sk/Graphics/Rendering/Frame_Buffer.h>
#include <sk/Graphics/Utils/RenderUtils.h>
#include <sk/Graphics/Utils/Shader_Link.h>
#include <sk/Input/Input.h>
#include <sk/Math/Types.h>
#include <sk/Platform/Time.h>
#include <sk/Platform/ImGui/ImGuiHelper.h>
#include <sk/Platform/Window/Window_Base.h>
#include <sk/Scene/Scene.h>
#include <sk/Scene/Components/LightComponent.h>
#include <sk/Scene/Components/MeshComponent.h>
#include <sk/Scene/Components/SpinComponent.h>
#include <sk/Scene/Managers/SceneManager.h>
#include <sk/Scene/Objects/CameraFlight.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <chrono>
#include <tuple>

#ifdef SKAPE_EDITOR_AVAILABLE

using namespace sk::Editor;

using namespace std::chrono_literals;

cEditor::cEditor()
{
    Input::setLogInputs( false );


    cAsset_Manager::init();

    m_main_window_ = Platform::CreateWindow( "Main Window", { 1280, 720 } );

    Graphics::InitRenderer();
    Graphics::Utils::InitUtils();
    m_main_window_->Init();

    SK_ERR_IFN( m_main_window_->SetVisibility( true ),
        "Unable to show window." )

    // TODO: Create a render context.


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    Gui::InitImGui( m_main_window_ );

    cSceneManager::init();
	Managers::cSelectionManager::init();
}

cEditor::~cEditor()
{
	Destroy();

	Managers::cSelectionManager::shutdown();

    cSceneManager::shutdown();

    Gui::ImGuiShutdown();
    ImGui::DestroyContext();

    SK_DELETE( m_main_window_ );

    cAsset_Manager::shutdown();

    // I don't really like the choice of having the renderer shut down after the asset system,
    // with the asset system being started before the renderer.
    // TODO: Add some RegisterRendererListeners function or something in the future.
    Graphics::Utils::ShutdownUtils();
    Graphics::cRenderer::shutdown();
    cStringIDManager::shutdown();
}

bool cEditor::IsGameRunning() const
{
    return m_is_game_running_.load();
}

void cEditor::Create()
{
    auto& asset_m = cAsset_Manager::get();

	asset_m.loadFolder( SK_ROOT_DIR );

	auto list_1 = asset_m.GetAssetsByPath( "models/humanforscale.glb" );
	auto list_2 = asset_m.GetAssetsByPath( "models/heheToiletwithtextures.glb" );
	const auto shader_frag = asset_m.GetAssetByPath( "shaders/gpass.frag" );
	const auto shader_vert = asset_m.GetAssetByPath( "shaders/default.vert" );

	sk::MakeShared< Tabs::cObjectListTab >( "Objects" );
	sk::ceval( sk::make_shared_helper{} ).impl< Tabs::cObjectListTab >( "Objects" );

	// TODO: Create a material instance class.

	// Testing Scene
	const auto christopher_t = list_1.GetAssetOfType< Assets::cTexture >();
	const auto toilet_t      = list_2.GetAssetOfType< Assets::cTexture >();

	auto mat1 = asset_m.CreateAsset< Assets::cMaterial >( "Material Test", "materials/mat1.skmat",
		Graphics::Utils::cShader_Link{ shader_vert, shader_frag } );
	mat1.second->SetTexture( "mainTexture", christopher_t );

	auto mat2 = asset_m.CreateAsset< Assets::cMaterial >( "Material Test 2", "materials/mat2.skmat",
		Graphics::Utils::cShader_Link{ shader_vert, shader_frag } );
	mat2.second->SetTexture( "mainTexture", toilet_t );


	/*
	auto christopher_m       = list_1.GetAssetOfType< Assets::cMesh    >();
	auto toilet_m            = list_2.GetAssetOfType< Assets::cMesh    >();

	auto [ scene_meta, scene ] = asset_m.CreateAsset< cScene >( "Main Scene", "scenes/main_scene.skscene" );
	scene->create_object< Object::cCameraFlight >( "Camera Free Flight" )->setAsMain();

	auto mesh = scene->create_object< Object::cObject >( "Mesh Test 2" );
	mesh->GetTransform().SetLocalPosition( { 0.0f, -16.0f, 90.0f } );
	auto component = mesh->AddComponent< Object::Components::cMeshComponent >( christopher_m, mat1.first );
	component->SetScale( cVector3f{ 100.0f } );

	mesh = scene->create_object< Object::cObject >( "Mesh Test 3" );
	mesh->GetTransform().SetLocalPosition( { -0.1f, 0.0f, 2.5f } );
	auto spin_component = mesh->AddComponent< Object::Components::cSpinComponent >( cVector3f{ 0.0f, 5.0f, 0.0f } );
	component = mesh->AddComponent< Object::Components::cMeshComponent >( toilet_m, mat2.first );
	component->SetParent( spin_component );

	Scene::Light::sSettings light_settings{};
	auto light_object = scene->create_object< Object::cObject >( "Directional Light" );
	light_settings.shadow_resolution = 1024;
	light_settings.casts_shadows     = true;
	auto directional_light = light_object->AddComponent< Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );

	light_object = scene->create_object< Object::cObject >( "Point Light" );
	light_settings.type              = Scene::Light::eType::kPoint;
	light_settings.casts_shadows     = false;
	light_settings.shadow_resolution = 256;
	auto point_light = light_object->AddComponent< Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );

	light_object = scene->create_object< Object::cObject >( "Spot Light 1" );
	light_settings.type              = Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	auto spot_light = light_object->AddComponent< Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 0.0f, 0.0f } );
	light_object->GetTransform().SetLocalPosition( { 0.0f, 5.0f, 0.0f } );

	light_object = scene->create_object< Object::cObject >( "Spot Light 2" );
	light_settings.type              = Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	spot_light = light_object->AddComponent< Object::Components::cLightComponent >( light_settings );
	light_object->GetTransform().SetLocalRotation( { -45.0f, 90.0f, 0.0f } );
	light_object->GetTransform().SetLocalPosition( { 0.0f, 5.0f, 0.0f } );

	light_object = scene->create_object< Object::cObject >( "Spot Light 3" );
	light_settings.type              = Scene::Light::eType::kSpot;
	light_settings.casts_shadows     = true;
	light_settings.shadow_resolution = 512;
	spot_light = light_object->AddComponent< Object::Components::cLightComponent >( light_settings );
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
			auto mesh_object = scene->create_object< Object::cObject >( "Clone" );
			mesh_object->GetTransform().SetLocalPosition( { x * 2, 0.0f, y * 2 } );
			auto spin = mesh_object->AddComponent< Object::Components::cSpinComponent >( cVector3f{ 0.0f, dis( gen ), 0.0f } );
			auto mesh_component = mesh_object->AddComponent< Object::Components::cMeshComponent >( christopher_m, mat1.first );
			mesh_component->SetRotation( { -90.0f, 0.0f, 0.0f } );
			mesh_component->SetParent( spin );
		}
	}

	/*
	constexpr auto runs = 10;

	auto start = std::chrono::high_resolution_clock::now();
	double fastest_time = std::numeric_limits< double >::max();
	double slowest_time = std::numeric_limits< double >::min();

	for( size_t i = 0; i < runs; ++i )
	{
		auto local_start = std::chrono::high_resolution_clock::now();
		scene_meta->Save();
		auto spent = std::chrono::duration_cast< std::chrono::duration< double, std::milli > >( std::chrono::high_resolution_clock::now() - local_start ).count();
		if( spent < fastest_time )
			fastest_time = spent;
		if( spent > slowest_time )
			slowest_time = spent;
	}

	auto total_spent = std::chrono::duration_cast< std::chrono::duration< double, std::milli > >( std::chrono::high_resolution_clock::now() - start ).count();
	double average_time = total_spent / runs;

	sk::println( "Runs: {}    Total Time: {}    Average Time: {}    Fastest Time: {}    Slowest Time: {}", runs, total_spent, average_time, fastest_time, slowest_time );

	cSceneManager::get().RegisterScene( scene_meta );
	//*/

    cSceneManager::get().update();

	addTab( std::make_unique< Tabs::cObjectListTab >( "Objects" ) );
	addTab( std::make_unique< Tabs::cPlaceholderTab >( "Properties" ) );
	addTab( std::make_unique< Tabs::cAssetGridViewTab >( "Assets" ) );
	addTab( std::make_unique< Tabs::cSceneViewportTab >( "Scene" ) );

	for( auto& tab : m_tabs_ )
		tab->Create();
}

void cEditor::Run()
{
    m_main_window_->UseContext();

    Gui::ImGuiNewFrame();

    Time::Update();
    Graphics::cRenderer::get().Update();

    cSceneManager::get().update();
	cEventManager::get().postEvent( m_is_game_running_.load() ? Object::kUpdate : Object::kEditorUpdate );

    _drawMainWindow();

	for( const auto& tab : m_tabs_ )
	{
		if( ImGui::Begin( tab->m_name_with_id_.c_str() ) )
			tab->Draw();
		ImGui::End();
	}

	Managers::cSelectionManager::get().Clean();

    Gui::ImGuiRender();
    m_main_window_->SwapBuffers();

    Gui::ImGuiUpdateWindows();
}

void cEditor::Destroy()
{
	for( auto& asset : cAsset_Manager::get().GetAllAssets() | std::views::values )
		asset->Save();

	std::this_thread::sleep_for( 2s );

	for( const auto& tab : m_tabs_ )
		tab->Destroy();
	m_tabs_.clear();
}

void cEditor::_drawMainWindow()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    static ImGuiID main_viewport_id = ImGui::GetMainViewport()->ID;

    const ImGuiViewport* viewport = ImGui::FindViewportByID( main_viewport_id );
    ImGui::SetNextWindowPos( viewport->WorkPos );
    ImGui::SetNextWindowSize( viewport->WorkSize );
    ImGui::SetNextWindowViewport( main_viewport_id );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    bool open = true;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f } );
    ImGui::Begin("Test", &open, window_flags );
    ImGui::PopStyleVar();
    ImGui::PopStyleVar( 2 );

    auto dockspace_id = ImGui::GetID( "RootDockspace" );

    if( ImGui::DockBuilderGetNode( dockspace_id ) == nullptr )
    {
        ImGui::DockBuilderRemoveNode( dockspace_id );
        ImGui::DockBuilderAddNode( dockspace_id );

        ImGui::DockBuilderFinish( dockspace_id );
    }

	if( ImGui::Button( m_is_game_running_.load() ? "Pause" : "Play" ) )
		m_is_game_running_.store( !m_is_game_running_.load() );

    ImGui::DockSpace( dockspace_id );
    ImGui::End();
}

void cEditor::addTab( std::unique_ptr< Tabs::aTab >&& _tab )
{
	auto& tab = *_tab;
	tab.m_id_ = m_tabs_.size();
	tab.m_name_with_id_ = tab.m_name_ + "##" + std::to_string( tab.m_id_ );

	m_tabs_.emplace_back( std::move( _tab ) );
}

#endif // SKAPE_EDITOR_AVAILABLE
