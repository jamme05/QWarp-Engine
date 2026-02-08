/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Containers/Map.h>
#include <sk/Containers/Vector.h>
#include <sk/Misc/Smart_Ptrs.h>
#include <sk/Misc/StringID.h>
#include <sk/Reflection/RuntimeClass.h>
#include <sk/Scene/Components/Component.h>
#include <sk/Scene/Components/TransformComponent.h>
#include <sk/Scene/Managers/Internal_Component_Manager.h>

#include <ranges>

namespace sk::Object::Components
{
	class cMeshComponent;
} // sk::Object::Components::

namespace sk
{
	class cSceneManager;
	class cScene;
} // sk::

namespace sk::Object
{
// TODO: Check if class shit is needed? Like with Assets and Components.
	GENERATE_CLASS( cObject ), public Event::cEventListener, public cShared_from_this< cObject >, public iSerializable
	{
		CREATE_CLASS_BODY( cObject )

		friend class sk::cScene;
		friend class sk::cSceneManager;
	sk_public:
		// TODO: Create templated constructor with root type + parameters
		explicit cObject( const std::string& _name ); // iObject
		explicit cObject( cSerializedObject& _object );

		template< class Ty = iComponent, class... Args >
		explicit cObject( const std::string& _name, Args... _args )
		: m_root( sk::MakeShared< Ty >( _args... ) )
		, m_name( _name )
		{} // iObject

		~cObject() override;

		template< class Ty, class... Args >
		requires ( std::is_base_of_v< iComponent, Ty > && std::constructible_from< Ty, Args... > )
		auto AddComponent( Args&&... _args ) -> cShared_ptr< Ty >
		{
			auto component = sk::MakeShared< Ty >( std::forward< Args >( _args )... );

			component->m_object_ = get_weak();
			component->m_uuid_   = GenerateRandomUUID();
			component->SetParent( m_root );

			if constexpr( std::is_base_of_v< Components::cMeshComponent, Ty > )
				m_mesh_components_.emplace_back( component );
			else
				m_components_.insert( std::pair{ Ty::getStaticType(), component } );

			return component;
		} // addComponent

		void AddComponent( const cShared_ptr< iComponent >& _component );

		// TODO: Have get component actually get a component instead of creating one.
		template< class Ty >
		requires std::is_base_of_v< iComponent, Ty >
		auto GetComponent() -> cShared_ptr< Ty >
		{
			cShared_ptr< Ty > component = nullptr;
			if constexpr( std::is_base_of_v< Components::cMeshComponent, Ty > )
			{
				if( !m_mesh_components_.empty() )
					component = m_mesh_components_.front().Cast< Ty >();
			}
			else
			{
				if( auto itr = m_components_.find( Ty::getStaticType() ); itr != m_components_.end() )
					component = itr->second.template Cast< Ty >();
			}
			return component;
		} // addComponent

		void RemoveComponent( const cShared_ptr< iComponent >& _component );

		// TODO: Add a way to remove the component

		/**
		 * 
		 * @tparam Ty The type of the component, has to inherit iComponent and have a default constructor.
		 * @param _requested_index [in/out] The index that the component is expected to be at,
		 * otherwise have the value as std::numeric_limits< size_t >::max() and it'll be set to the correct index.
		 * When calling this it is recommended to store the requested index.
		 * NOTE: Doing the first call without having this as std::numeric_limits< size_t >::max() is undefined behavior.
		 * @return A pair where first is if the component was created this call, second is the component itself.
		 * 
		 * Internal components are hidden by the editor and are mainly used to contain additional data on the object for systems.
		 * You are only allowed to have a single internal component of a type at a time.
		 * Due to being able to get the component without reconstructing it, it will be limited to using the default constructor.
		 * Along with this you will have to verify that all values are valid.
		 * ALSO, it isn't recommended to reuse the same internal component between multiple systems as it can cause collisions.
		 * 
		 */
		template< class Ty >
		requires ( std::is_base_of_v< iComponent, Ty > && std::is_default_constructible_v< Ty >)
		auto AddOrGetInternalComponent( size_t& _requested_index ) -> std::pair< bool, cShared_ptr< Ty > >
		{
			if( _requested_index == std::numeric_limits< size_t >::max() )
				_requested_index = Scene::cInternal_Component_Manager::get().GetComponentIndex< Ty >();
			
			if( m_internal_components_.size() <= _requested_index )
				m_internal_components_.resize( _requested_index + 1 );
			
			if( auto& component = m_internal_components_[ _requested_index ]; component != nullptr )
			{
				if( dynamic_cast< Ty* >( component.get() ) != nullptr )
					return { false, component.Cast< Ty >() };
				
				SK_WARNING( sk::Severity::kEngine, "The requested index is currently trying to access another types internal component." )
				SK_BREAK;
				
				_requested_index = std::numeric_limits< size_t >::max();
				return AddOrGetInternalComponent< Ty >( _requested_index );
			}
			
			auto component = sk::MakeShared< Ty >();

			component->m_object_   = get_weak();
			component->m_uuid_     = GenerateRandomUUID();
			component->m_internal_ = true;
			component->SetParent( m_root );
			
			m_internal_components_[ _requested_index ] = component;

			return { true, component };
		} // addComponent

		// TODO: Deprecate?
		virtual void render();
		// render

		virtual void update();
		// update

		void SetLayer( uint64_t _layer );
		[[ nodiscard ]]
		auto  GetLayer() const { return m_layer_; }

		[[ nodiscard ]] auto& GetUUID() const { return m_uuid_; }
		[[ nodiscard ]] auto& GetRoot()       { return m_root; }
		[[ nodiscard ]] auto& GetRoot() const { return m_root; }

		[[ nodiscard ]] auto& GetChildren() const { return m_children_; }

		[[ nodiscard ]] auto& GetComponents() const { return m_components_; }
		[[ nodiscard ]] auto& GetMeshComponents() const { return m_mesh_components_; }

		[[ nodiscard ]] auto& GetPosition()       { return m_root->GetLocalPosition(); }
		[[ nodiscard ]] auto& GetPosition() const { return m_root->GetLocalPosition(); }

		[[ nodiscard ]] auto& GetRotation()       { return m_root->GetLocalRotation(); }
		[[ nodiscard ]] auto& GetRotation() const { return m_root->GetLocalRotation(); }

		[[ nodiscard ]] auto& GetScale()       { return m_root->GetLocalScale(); }
		[[ nodiscard ]] auto& GetScale() const { return m_root->GetLocalScale(); }

		[[ nodiscard ]] auto& GetTransform()       { return m_root->GetTransform(); }
		[[ nodiscard ]] auto& GetTransform() const { return m_root->GetTransform(); }

		[[ nodiscard ]] auto& GetName() const { return m_name; }

		auto Serialize() -> cSerializedObject override;

	sk_protected:
		void SetRoot( const cShared_ptr< iComponent >& _new_root_component, bool _override_parent = false );

		void registerRecursive();
		void enableRecursive ();
		void disableRecursive();

	sk_private:
		cShared_ptr< iComponent > m_root;
		cUUID m_uuid_;

		// TODO: Use typedefs/using
		vector             < cShared_ptr< cObject > >              m_children_   = { };
		unordered_multimap< type_hash, cShared_ptr< iComponent > > m_components_ = { };
		// Internal components are stored separately as there can only be one of a type.
		vector< cShared_ptr< iComponent >  > m_internal_components_ = { };
		// We have the mesh components separately to allow for a faster lookup.
		vector< cShared_ptr< Components::cMeshComponent > >        m_mesh_components_;

		cStringID m_name;

		// TODO: Make this into a weak ptr.
		cScene* m_parent_scene = nullptr;

		// TODO: Actually implament these. Follow Unity's design when it comes to layers. But do allow multiple tags.
		std::vector< str_hash > m_tags_;
		uint64_t                m_layer_ = 1;
	};

	namespace Object
	{
		using class_type = cObject;
	} // sk::Object::

} // sk::Object::

SK_DECLARE_CLASS( sk::Object::Object )

#define OBJECT_PARENT_CLASS( ObjectName, ... ) sk::Object::cObject
#define OBJECT_PARENT_VALIDATOR( ObjectName, ... ) std::is_base_of_v< sk::Object::cObject, __VA_ARGS__ >
#define OBJECT_PARENT_CREATOR_2( ObjectName, ... ) AFTER_FIRST( __VA_ARGS__ )
#define OBJECT_PARENT_CREATOR_1( ObjectName, ... ) cObject
#define OBJECT_PARENT_CREATOR( ObjectName, ... ) CONCAT( OBJECT_PARENT_CREATOR_, VARGS( __VA_ARGS__ ) ) ( ObjectName, __VA_ARGS__ )
#define SK_OBJECT_CLASS( ObjectName, ... ) QW_RESTRICTED_CLASS( ObjectName, OBJECT_PARENT_CLASS, OBJECT_PARENT_CREATOR, OBJECT_PARENT_VALIDATOR, EMPTY __VA_OPT__( , __VA_ARGS__ ) )
// TODO: Rename to SK_OBJECT_CLASS

#define OBJECT_CLASS_PARENT( ObjectName )
//#define QW_OBJECT_CLASS( ObjectName ) QW_CLASS( ObjectName, OBJECT_CLASS_PARENT, EMPTY )