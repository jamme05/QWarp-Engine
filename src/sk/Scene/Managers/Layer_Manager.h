

#pragma once

#include <sk/Misc/Hashing.h>
#include <sk/Misc/Singleton.h>
#include <sk/Misc/Smart_Ptrs.h>
#include <sk/Misc/StringID.h>
#include <sk/Scene/Object.h>

#include <unordered_map>

namespace sk::Object::Components
{
    class cMeshComponent;
} // sk::Object::Components

namespace sk::Scene
{
    class cLayer_Manager : public cSingleton< cLayer_Manager >
    {
    public:
        using object_vec_t = vector< cWeak_Ptr< Object::cObject > >;
        
        struct sLayer
        {
            cStringID    name;
            uint64_t     layer;
            object_vec_t objects;
        };
        
        class cObjectIterator
        {
        public:
            using layer_vec_t = std::vector< const sLayer* >;
            
            cObjectIterator() = default;
            cObjectIterator( layer_vec_t _layers, const size_t _layer_index, const size_t _object_index )
            : m_layers_{ std::move( _layers ) }
            , m_layer_index_{ _layer_index }
            , m_object_index_{ _object_index }
            {
            } // cLayerIterator
            
            using difference_type   = std::ptrdiff_t;
            using value_type        = object_vec_t::value_type;
            using pointer           = value_type*;
            using reference         = value_type&;
            using iterator_category = std::forward_iterator_tag;

            // TODO: Move logic to cpp
            auto& operator*() const
            {
                return getObject();
            }
            
            auto& operator->() const
            {
                return getObject();
            }
            
            auto operator++() -> cObjectIterator&;

            auto operator++( int )
            {
                auto tmp = *this;
                
                ++( *this );
                
                return tmp;
            }
            
            auto operator--() -> cObjectIterator&;

            auto operator--( int )
            {
                auto tmp = *this;
                
                --( *this );
                
                return tmp;
            }
            
            bool operator==( const cObjectIterator& _other ) const
            {
                return m_layer_index_ == _other.m_layer_index_ && m_object_index_ == _other.m_object_index_;
            }

        private:
            [[ nodiscard ]] auto getObjects() const -> const object_vec_t&
            {
                return m_layers_[ m_layer_index_ ]->objects;
            }
            
            [[ nodiscard ]] auto getObject() const -> const cWeak_Ptr< Object::cObject >&
            {
                return getObjects()[ m_object_index_ ];
            }
            
            layer_vec_t m_layers_;
            size_t      m_layer_index_;
            size_t      m_object_index_;
        };

        class cMeshIterator
        {
        public:
            cMeshIterator() = default;
            cMeshIterator( cObjectIterator _object_itr_start, cObjectIterator _object_itr_end, const size_t _mesh_index )
            : m_object_itr_{ std::move( _object_itr_start ) }
            , m_object_end_{ std::move( _object_itr_end ) }
            , m_mesh_index_{ _mesh_index }
            {
            } // cLayerIterator
            
            using difference_type   = std::ptrdiff_t;
            using value_type        = cShared_ptr< Object::Components::cMeshComponent >;
            using pointer           = value_type*;
            using reference         = value_type&;
            using iterator_category = std::forward_iterator_tag;

            // TODO: Handle empty layers. With them the object won't be valid.
            [[ nodiscard ]] bool IsValid() const
            {
                return getObject().is_valid() && !getMeshes().empty();
            }
            
            operator bool() const
            {
                return IsValid();
            }
            
            auto& operator*() const
            {
                auto& meshes = getMeshes();
                return meshes[ m_mesh_index_ ];
            }
            
            auto& operator->() const
            {
                auto& meshes = getMeshes();
                return meshes[ m_mesh_index_ ];
            }
            
            auto& operator++()
            {
                if( ++m_mesh_index_ >= getMeshes().size() )
                {
                    // This should cap the iterator at the end.
                    if( m_object_itr_ != m_object_end_ )
                        ++m_object_itr_;
                    m_mesh_index_ = 0;
                }
                
                return *this;
            }
            
            auto operator++( int )
            {
                auto tmp = *this;
                
                ++( *this );
                
                return tmp;
            }
            
            auto& operator--()
            {
                if( m_mesh_index_ == 0 )
                {
                    // This should cap the iterator at the end.
                    static auto zero_iterator = cObjectIterator{ {}, 0, 0 };
                    if( m_object_itr_ != zero_iterator )
                    {
                        --m_object_itr_;
                        m_mesh_index_ = getMeshes().size() - 1;
                    }
                }
                else
                    m_mesh_index_++;
                
                return *this;
            }
            
            auto operator--( int )
            {
                auto tmp = *this;
                
                --( *this );
                
                return tmp;
            }
            
            bool operator==( const cMeshIterator& _other ) const
            {
                return m_mesh_index_ == _other.m_mesh_index_ && m_object_itr_ == _other.m_object_itr_;
            }

        private:
            [[ nodiscard ]] auto getObject() const -> const cWeak_Ptr< Object::cObject >&
            {
                return *m_object_itr_;
            }
            
            [[ nodiscard ]] auto getMeshes() const -> const vector< cShared_ptr< Object::Components::cMeshComponent > >&
            {
                return getObject()->GetMeshComponents();
            }
            
            [[ nodiscard ]] auto getMesh() const -> const cShared_ptr< Object::Components::cMeshComponent >&
            {
                return getMeshes()[ m_mesh_index_ ];
            }
            
            cObjectIterator m_object_itr_;
            cObjectIterator m_object_end_;
            size_t          m_mesh_index_;
        };

        using object_range_t = std::pair< cObjectIterator, cObjectIterator >;
        using mesh_range_t   = std::pair< cMeshIterator, cMeshIterator >;
        
        cLayer_Manager();

        void AddLayer   ( uint8_t _layer, const cStringID& _name );
        void RemoveLayer( uint8_t _layer );

        void AddObject   ( const cShared_ptr< Object::cObject >& _object );
        void RemoveObject( const cShared_ptr< Object::cObject >& _object );

        void Lock();
        void Unlock();

        [[ nodiscard ]] auto  GetLayerByName ( const cStringID& _name   ) const -> const sLayer*;
        [[ nodiscard ]] auto  GetLayerByValue(       uint64_t   _layer  ) const -> const sLayer*;
        [[ nodiscard ]] auto  GetObjectsIn   (       uint64_t   _layers ) const -> object_range_t;
        [[ nodiscard ]] auto  GetMeshesIn    (       uint64_t   _layers ) const -> mesh_range_t;

        [[ nodiscard ]] auto& GetLayers() const { return m_layers_; }

    private:
        void removeObjectAt( size_t _layer_index, size_t _index );
        
        using hash_to_i_map_t = map< str_hash, uint64_t >;
        using layer_vec_t     = vector< sLayer >;

        std::mutex m_access_mtx_;

        size_t          m_internal_component_index_ = std::numeric_limits< size_t >::max();
        hash_to_i_map_t m_name_to_layer_;
        layer_vec_t     m_layers_;
    };
} // sk::Scene
