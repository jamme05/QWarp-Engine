

#include "LightComponent.h"

#include <sk/Math/Vector4.h>
#include <sk/Scene/Managers/CameraManager.h>
#include <sk/Scene/Managers/Light_Manager.h>

#include "CameraComponent.h"


using namespace sk::Object::Components;
using namespace sk::Scene::Light;

namespace 
{
    // TODO: Make an include for this
    // From: https://en.cppreference.com/w/cpp/utility/variant/visit
    template< class... Ts >
    struct sVisitor : Ts... { using Ts::operator()...; };
} // ::

cLightComponent::cLightComponent( const settings_t& _settings )
: m_settings_( _settings )
{
    init_data();
    fix_data();

    Scene::cLight_Manager::get().register_light( get_weak().Cast< cLightComponent >() );
}

cLightComponent::cLightComponent( cSerializedObject& _object )
: cComponent( _object.GetBase< iComponent >().value() )
{
    _object.BeginRead( this );
    m_settings_ = {
        .type              = _object.ReadValueOr( "type", eType::kDirectional ),
        .casts_shadows     = _object.ReadValueOr( "casts_shadows", false ),
        .shadow_resolution = _object.ReadValueOr< uint16_t >( "shadow_resolution", 256 ),
        .color             = _object.ReadValueOr< cVector4f >( "color", kOne ),
        .intensity         = _object.ReadValueOr( "intensity", 1.0f ),
        .radius            = _object.ReadValueOr( "radius", 10.0f ),
        .inner_angle       = _object.ReadValueOr( "inner_angle", 45.0f ),
        .outer_angle       = _object.ReadValueOr( "outer_angle", 60.0f ),
    };
    _object.EndRead();

    init_data();
    fix_data();

    Scene::cLight_Manager::get().register_light( get_weak().Cast< cLightComponent >() );
}

cLightComponent::~cLightComponent()
{
    if( const auto light_manager = Scene::cLight_Manager::getPtr() )
        light_manager->unregister_light( get_weak().Cast< cLightComponent >() );
}

void cLightComponent::update()
{
    if( GetType() == eType::kDirectional || m_transform_->IsDirty() )
    {
        m_transform_->Update();

        update_data();
    }
}

void cLightComponent::UpdateViewProjectionMatrix()
{
    switch( m_settings_.type )
    {
    case eType::kDirectional:
    {
        // TODO: Make the projection dynamic.
        auto main_camera = Scene::cCameraManager::get().getMainCamera();

        constexpr auto distance = 1000.0f;
        constexpr auto projection = Math::Matrix4x4::Ortho< float >( 100.0f, 100.0f, 1.0f, distance );



        auto sun_matrix = GetTransform().GetWorld();
        sun_matrix.position  = main_camera->GetTransform().GetPosition();
        sun_matrix.position += m_transform_->GetForwardDir() * ( -distance / 2.0f );
        sun_matrix.forward  *= -1.0f;
        m_view_proj_matrix_  = sun_matrix.inverse() * projection;
    }
    break;
    case eType::kPoint:
    {
        SK_BREAK;
    }
    break;
    case eType::kSpot:
    {
        const auto projection = Math::Matrix4x4::AspectPerspective< float >( 1.0f, m_settings_.outer_angle, 1.0f, 100.0f );
        m_transform_->GetWorld().inversed( m_view_proj_matrix_ );
        m_view_proj_matrix_ *= projection;
    }
    break;
    }
}

auto cLightComponent::GetSettings() const -> const settings_t&
{
    return m_settings_;
}

auto cLightComponent::GetType() const -> type_t
{
    return m_settings_.type;
}

#define ASSIGN_SETTING_0_( Setting, NewValue ) \
    const auto prev_settings = m_settings_;   \
    Setting = NewValue; \
    Scene::cLight_Manager::get().update_light( get_weak().Cast< cLightComponent >(), prev_settings ); \
    fix_data()

#define ASSIGN_SETTING( Setting, ... ) \
    ASSIGN_SETTING_0_( FIRST( __VA_OPT__( m_settings_ . Setting , ) m_settings_ ), FIRST( __VA_OPT__( __VA_ARGS__, ) Setting ) )

void cLightComponent::SetSettings( const settings_t& _settings )
{
    ASSIGN_SETTING( _settings );
}

void cLightComponent::SetType( const type_t _type )
{
    if( GetType() != _type )
    {
        init_data();
        fix_data();
        
        ASSIGN_SETTING( type, _type );
    }
}

auto cLightComponent::GetData() const -> const data_t&
{
    return m_data_;
}

auto cLightComponent::GetShadowCasterData() const -> const caster_t*
{
    return m_shadow_data_index_ != std::numeric_limits< uint32_t >::max() ? &Scene::cLight_Manager::get().m_shadow_caster_buffer_[ m_shadow_data_index_ ] : nullptr;
}

auto cLightComponent::Serialize() -> cSerializedObject
{
    cSerializedObject object( this );
    object.AddBase( cComponent::Serialize() );
    object.BeginWrite( this );
    object.WriteValue( "type", GetType() );
    object.WriteValue( "casts_shadows", m_settings_.casts_shadows );
    object.WriteValue( "shadow_resolution", m_settings_.shadow_resolution );
    object.WriteValue( "color", cVector4d{ m_settings_.color } );
    object.WriteValue( "intensity", m_settings_.intensity );
    if( GetType() == eType::kPoint || GetType() == eType::kSpot )
        object.WriteValue( "radius", m_settings_.radius );
    if( GetType() == eType::kSpot )
    {
        object.WriteValue( "inner_angle", m_settings_.inner_angle );
        object.WriteValue( "outer_angle", m_settings_.outer_angle );
    }
    object.EndWrite();
    return object;
}

auto cLightComponent::GetViewProjMatrix() const -> const cMatrix4x4f&
{
    return m_view_proj_matrix_;
}

void cLightComponent::fix_data()
{
    auto& manager = Scene::cLight_Manager::get();
    manager.mark_buffer_dirty( GetType() );
    
    switch( GetType() )
    {
    case eType::kDirectional:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sDirectionalLight >( m_data_ )
            : manager.m_directional_buffer_[ m_data_index_ ];

        fix_directional_data( data );
    }
    break;
    case eType::kPoint:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sPointLight >( m_data_ )
            : manager.m_point_buffer_[ m_data_index_ ];

        fix_point_data( data );
    }
    break;
    case eType::kSpot:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sSpotLight >( m_data_ )
            : manager.m_spot_buffer_[ m_data_index_ ];

        fix_spot_data( data );
    }
    break;
    }
}

void cLightComponent::fix_directional_data( sDirectionalLight& _data ) const
{
    _data.color     = m_settings_.color * m_settings_.intensity;
    _data.direction = GetTransform().GetForward();
    _data.shadow_cast_index = -1;
}

void cLightComponent::fix_point_data( sPointLight& _data ) const
{
    _data.color    = m_settings_.color * m_settings_.intensity * m_settings_.color.a;
    _data.position = GetTransform().GetPosition();
    _data.radius   = m_settings_.radius;
    std::ranges::fill( _data.shadow_cast_indices, -1 );
}

void cLightComponent::fix_spot_data( sSpotLight& _data ) const
{
    _data.color       = m_settings_.color * m_settings_.intensity * m_settings_.color.a;
    _data.position    = GetTransform().GetPosition();
    _data.direction   = GetTransform().GetForward();
    _data.inner_angle = m_settings_.inner_angle;
    _data.outer_angle = m_settings_.outer_angle;
    _data.shadow_cast_index = -1;
}

void cLightComponent::update_data()
{
    auto& manager = Scene::cLight_Manager::get();
    
    auto& transform = GetTransform();
    auto& settings  = m_settings_;
    switch( m_settings_.type )
    {
    case eType::kDirectional:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sDirectionalLight >( m_data_ )
            : manager.m_directional_buffer_[ m_data_index_ ];

        data.direction = transform.GetForward();
    }
    break;
    case eType::kPoint:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sPointLight >( m_data_ )
            : manager.m_point_buffer_[ m_data_index_ ];

        data.position = GetTransform().GetPosition();
    }
    break;
    case eType::kSpot:
    {
        auto& data = m_data_index_ == std::numeric_limits< uint32_t >::max()
            ? std::get< sSpotLight >( m_data_ )
            : manager.m_spot_buffer_[ m_data_index_ ];

        data.position  = transform.GetPosition();
        data.direction = transform.GetForward() * settings.radius;
    }
    break;
    }

    manager.mark_buffer_dirty( GetType() );
    
    if( m_shadow_data_index_ != std::numeric_limits< uint32_t >::max() )
    {
        UpdateViewProjectionMatrix();
        manager.m_shadow_caster_buffer_[ m_shadow_data_index_ ].light_view_proj = GetViewProjMatrix();
        manager.mark_shadow_buffer_dirty( true );
    }
}

void cLightComponent::init_data()
{
    switch( m_settings_.type )
    {
    case eType::kDirectional:
        m_data_ = sDirectionalLight{};
        break;
    case eType::kPoint:
        m_data_ = sPointLight{};
        break;
    case eType::kSpot:
        m_data_ = sSpotLight{};
        break;
    }
}
