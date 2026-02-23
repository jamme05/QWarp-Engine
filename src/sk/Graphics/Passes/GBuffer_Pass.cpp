

#include "GBuffer_Pass.h"

#include <framework/App.h>
#include <sk/Debugging/Macros/Assert.h>
#include <sk/Graphics/Pipelines/Pipeline.h>
#include <sk/Graphics/Rendering/Depth_Target.h>
#include <sk/Graphics/Rendering/Frame_Buffer.h>
#include <sk/Graphics/Utils/RenderUtils.h>
#include <sk/Platform/Window/Window_Base.h>
#include <sk/Scene/Components/CameraComponent.h>
#include <sk/Scene/Components/MeshComponent.h>
#include <sk/Scene/Managers/CameraManager.h>
#include <sk/Scene/Managers/Layer_Manager.h>

using namespace sk::Graphics::Passes;

void cGBuffer_Pass::Init()
{
    m_render_context_ = std::make_unique< Rendering::cRender_Context >( 2, 3 );

    m_resolution_ = getPipeline().GetSurface()->GetResolution();

    for( const auto& frame_buffer : *m_render_context_ )
    {
        auto color_target    = sk::MakeShared< Rendering::cRender_Target >( m_resolution_, Rendering::cRender_Target::eFormat::kRGBA8 )();
        auto position_target = sk::MakeShared< Rendering::cRender_Target >( m_resolution_, Rendering::cRender_Target::eFormat::kRGBA16F )();
        auto normal_target   = sk::MakeShared< Rendering::cRender_Target >( m_resolution_, Rendering::cRender_Target::eFormat::kRGBA16F )();
        
        auto depth_target = sk::MakeShared< Rendering::cDepth_Target >( m_resolution_, Rendering::cDepth_Target::eFormat::kD24FS8 )();
        
        frame_buffer->Bind( 0, position_target );
        frame_buffer->Bind( 1, normal_target );
        frame_buffer->Bind( 2, color_target );
        
        frame_buffer->Bind( depth_target );
    }
}

bool cGBuffer_Pass::Begin()
{
    const auto  surface        = getPipeline().GetSurface();
    const auto& camera_manager = Scene::cCameraManager::get();
    const auto& main_camera    = *camera_manager.getMainCamera();
    
    if( m_render_context_->GetBack().GetDepthTarget()->GetResolution() != surface->GetResolution() )
    {
        m_resolution_ = surface->GetResolution();
        m_render_context_->GetBack().Resize( m_resolution_ );
    }

    RenderWithCamera( main_camera );
    
    return true;
}

void cGBuffer_Pass::End()
{
    m_render_context_->End();
}

void cGBuffer_Pass::Destroy()
{
    m_render_context_.reset();
}

void cGBuffer_Pass::RenderWithCamera( const Object::Components::cCameraComponent& _camera ) const
{
    auto& layer_manager = Scene::cLayer_Manager::get();
    
    auto& frame_buffer = m_render_context_->GetBack();
    
    frame_buffer.Clear( Rendering::eClear::kAll );
    frame_buffer.Begin( _camera.getViewport(), _camera.getScissor() );

    layer_manager.Lock();
    for( auto [ fst, lst ] = layer_manager.GetMeshesIn( _camera.GetLayers() ); fst != lst; ++fst )
    {
        if( !fst.IsValid() )
            continue;

        auto& mesh = *fst;

        if( !mesh->IsReady() )
            continue;

        // TODO: Add a local render function that's more optimized
        const bool res = Utils::RenderMesh( _camera.getViewProjInv(), frame_buffer,
            *mesh->GetMaterial(), mesh->GetTransform().GetWorld(), *mesh->GetMesh() );
        if( !res )
            SK_BREAK;
    }
    layer_manager.Unlock();
}
