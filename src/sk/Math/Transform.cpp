
#include "Transform.h"

#include <sk/Seralization/SerializedObject.h>

using namespace sk;

cTransform::cTransform( cVector3f _position, cVector3f _rotation, cVector3f _scale )
: m_position_( std::move( _position ) )
, m_rotation_( std::move( _rotation ) )
, m_scale_   ( std::move( _scale ) )
{
    Update();
}

cTransform::cTransform( cSerializedObject& _object )
{
    _object.BeginRead();
    m_position_ = _object.ReadValue< cVector3f >( "Position" ).value();
    m_rotation_ = _object.ReadValue< cVector3f >( "Rotation" ).value();
    m_scale_    = _object.ReadValue< cVector3f >( "Scale" ).value();
    _object.EndRead();

    Update();
}

void cTransform::SetLocalPosition( const cVector3f& _position )
{
    m_position_ = _position;
    
    MarkDirty();
}

void cTransform::SetLocalRotation( const cVector3f& _rotation )
{
    m_rotation_ = _rotation;
    
    MarkDirty();
}

void cTransform::SetLocalScale( const cVector3f& _scale )
{
    m_scale_ = _scale;
    
    MarkDirty();
}

auto cTransform::GetRightDir() const -> const cVector3f&
{
    const_cast< cTransform* >( this )->CacheNormalized();

    return m_world_right_;
}

auto cTransform::GetUpDir() const -> const cVector3f&
{
    const_cast< cTransform* >( this )->CacheNormalized();

    return m_world_up_;
}

auto cTransform::GetForwardDir() const -> const cVector3f&
{
    const_cast< cTransform* >( this )->CacheNormalized();

    return m_world_forward_;
}

void cTransform::SetParent( const cWeak_Ptr< cTransform >& _parent )
{
    m_parent_ = _parent;
    
    MarkDirty();
}

bool cTransform::IsDirty( const bool _recursive ) const
{
    // Should we keep this function const, even if we cache the result using const cast?
    // To the user this should act and look as a getter.
    // But caching will make it significantly more efficient as we skip the recursion.

    if( m_is_dirty_ )
        return true;

    // TODO: Figure out if we can skip the recursive check,
    // maybe have a forced transform update during render instead of update
    // and during that time we allow it to be locked as dirty without checking recursively multiple times
    
    if( _recursive && m_parent_.is_valid() )
    {
        // Cache the result
        const_cast< bool& >( m_is_dirty_ ) = m_parent_->IsDirty();
        return m_is_dirty_;
    }
    
    return false;
}

void cTransform::MarkDirty()
{
    m_is_dirty_ = true;
}

void cTransform::Update( const bool _force )
{
    // We skip the IsDirty call on this object as we will reset m_is_dirty_ in the end anyway.
    // This method will be slightly more efficient.
    
    const bool parent_dirty = m_parent_.is_valid() && m_parent_->IsDirty();
    
    // Already up to date
    if( !_force && !m_is_dirty_ && !parent_dirty )
        return;
    
    const auto local = Math::Matrix4x4::scale_rotate_translate(
        m_scale_, m_rotation_ * Math::kDegToRad< float >, m_position_ );

    if( m_parent_.is_valid() )
    {
        if( parent_dirty )
            m_parent_->Update();
        m_world_ = local * m_parent_->GetWorld();
    }
    else
        m_world_ = local;
    
    m_is_dirty_       = false;
    m_has_normalized_ = false;
}

void cTransform::CacheNormalized()
{
    if( m_has_normalized_ )
        return;

    m_world_right_   = m_world_.right.normalized();
    m_world_up_      = m_world_.up.normalized();
    m_world_forward_ = m_world_.forward.normalized();

    m_has_normalized_ = true;
}

auto cTransform::Serialize() -> cSerializedObject
{
    cSerializedObject object{};
    object.BeginWrite();
    object.WriteValue( "Position", cVector3d( m_position_ ) );
    object.WriteValue( "Rotation", cVector3d( m_rotation_ ) );
    object.WriteValue( "Scale",    cVector3d( m_scale_ ) );
    object.EndWrite();

    return object;
}
