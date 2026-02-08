
#include "Layer_Manager.h"

#include <sk/Scene/Components/Internal/Layer_Component.h>

auto sk::Scene::cLayer_Manager::cObjectIterator::operator++() -> cObjectIterator&
{
    if( ++m_object_index_ >= getObjects().size() )
    {
        m_layer_index_++;
        m_object_index_ = 0;
    }
                
    return *this;
}

auto sk::Scene::cLayer_Manager::cObjectIterator::operator--() -> cObjectIterator&
{
    if( m_object_index_ == 0 )
    {
        // This should cap the iterator at the end.
        if( m_layer_index_ > 0 )
        {
            m_layer_index_--;
            m_object_index_ = getObjects().size() - 1;
        }
    }
    else
        m_object_index_--;
                
    return *this;
}

sk::Scene::cLayer_Manager::cLayer_Manager()
{
    AddLayer( 0, "Default" );
}

void sk::Scene::cLayer_Manager::AddLayer( const uint64_t _layer, const cStringID& _name )
{
    SK_BREAK_RET_IF( sk::Severity::kGeneral,
        m_name_to_layer_.contains( _name.hash() ),
        TEXT( "Layer with name {} has already been added.", _name.view() ) )
    
    SK_BREAK_RET_IFN( sk::Severity::kGeneral, Memory::is_power_of_two( _layer ),
        TEXT( "Layer with name {} and value {} is not a power of two.", _name.view(), _layer ) )
    
    sLayer* layer;
    if( m_layers_.size() <= static_cast< size_t >( _layer ) )
    {
        m_layers_.resize( _layer + 1 );
        layer = &m_layers_.back();
    }
    else
        layer = &m_layers_[ _layer ];
    
    m_name_to_layer_.emplace( std::pair{ _name.hash(), _layer } );

    // This may overwrite a layer in the same spot. But that will usually be intentional if done in the editor.
    // May want to consider triggering a break or warning in case it happens during runtime.
    auto& [ name, layer_val, objects ] = *layer;
    name      = _name;
    layer_val = 1 << _layer;
    objects   = object_vec_t{};
}

void sk::Scene::cLayer_Manager::RemoveLayer( const uint64_t _layer )
{
    SK_BREAK_RET_IF( sk::Severity::kGeneral, m_layers_.size() <= _layer,
        TEXT( "Layer with value {} does not exist.", _layer ) )
    
    auto& [ name, layer, objects ] = m_layers_[ _layer ];
    
    m_name_to_layer_.erase( name.hash() );
    
    name    = cStringID{};
    layer   = 0;
    objects = {};
}

void sk::Scene::cLayer_Manager::AddObject( const cShared_ptr< Object::cObject >& _object )
{
    std::scoped_lock lock{ m_access_mtx_ };

    auto [ was_created, component ] = _object->AddOrGetInternalComponent< Object::Components::cLayer_Info_Component >( m_internal_component_index_ );
    if( !was_created && component->m_index_ != std::numeric_limits< uint64_t >::max() )
        removeObjectAt( component->m_layer_index_, component->m_index_ );
    
    const auto layer_index = std::countr_one( _object->GetLayer() );
    auto& objects = m_layers_[ layer_index ].objects;
    
    component->m_layer_index_ = layer_index;
    component->m_index_       = objects.size();
    objects.emplace_back( _object );
}

void sk::Scene::cLayer_Manager::RemoveObject( const cShared_ptr< Object::cObject >& _object )
{
    std::scoped_lock lock{ m_access_mtx_ };

    auto [ was_created, component ] = _object->AddOrGetInternalComponent< Object::Components::cLayer_Info_Component >( m_internal_component_index_ );
    if( was_created )
        return;
    
    removeObjectAt( component->m_layer_index_, component->m_index_ );

    component->m_layer_index_ = std::numeric_limits< uint64_t >::max();
    component->m_index_       = std::numeric_limits< uint64_t >::max();
}

void sk::Scene::cLayer_Manager::Lock()
{
    m_access_mtx_.lock();
}

void sk::Scene::cLayer_Manager::Unlock()
{
    m_access_mtx_.unlock();
}

auto sk::Scene::cLayer_Manager::GetLayerByName( const cStringID& _name ) const -> const sLayer*
{
    if( const auto itr = m_name_to_layer_.find( _name.hash() ); itr != m_name_to_layer_.end() )
        return &m_layers_[ itr->second ];
    return nullptr;
}

auto sk::Scene::cLayer_Manager::GetLayerByValue( uint64_t _layer ) const -> const sLayer*
{
    SK_WARN_IF_RET( sk::Severity::kEngine, _layer >= m_layers_.size(),
        TEXT( "Layer with value {} does not exist.", _layer ), nullptr )

    auto& layer = m_layers_[ _layer ];

    SK_WARN_IF_RET( sk::Severity::kEngine, layer.layer != ( 1 << _layer ),
        TEXT( "Layer with value {} does not exist.", _layer ), nullptr )

    return &layer;
}

auto sk::Scene::cLayer_Manager::GetObjectsIn( const uint64_t _layers ) const -> object_range_t
{
    std::vector< const sLayer* > layers;
    for( uint32_t shift = 0; m_layers_.size() > shift; ++shift )
    {
        if( ( _layers & ( 1 << shift ) ) == 0 )
            continue;
        
        layers.emplace_back( &m_layers_[ shift ] );
    }
    
    cObjectIterator itr_start = { layers, 0, 0 };
    cObjectIterator itr_end   = { layers, layers.size(), 0 };
    
    return { itr_start, itr_end };
}

auto sk::Scene::cLayer_Manager::GetMeshesIn( const uint64_t _layers ) const -> mesh_range_t
{
    const auto [ fst, snd ] = GetObjectsIn( _layers );
    
    cMeshIterator mesh_itr_start = { fst, snd, 0 };
    cMeshIterator mesh_itr_end   = { snd, snd, 0 };
    
    return { mesh_itr_start, mesh_itr_end };
}

void sk::Scene::cLayer_Manager::
removeObjectAt( const size_t _layer_index, const size_t _index )
{
    // Make sure that the index is correct beforehand.
    
    auto& objects = m_layers_[ _layer_index ].objects;
    
    // If it's already the last element, we skip moving it
    if( _index < objects.size() - 1 )
    {
        auto& target = objects[ _index ];
        auto& back   = objects.back();
    
        std::swap( target, back );
    }
    
    objects.pop_back();
}

