/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <sk/Math/Matrix4x4.h>
#include <sk/Math/Transform.h>
#include <sk/Misc/Hashing.h>
#include <sk/Misc/Smart_Ptrs.h>
#include <sk/Misc/UUID.h>
#include <sk/Reflection/RuntimeClass.h>
#include <sk/Scene/Managers/EventManager.h>
#include <sk/Scene/Objects/SceneItem.h>
#include <sk/Seralization/SerializedObject.h>

namespace sk
{
	class cSceneManager;
} // sk::

namespace sk::Object
{
	class cObject;
} // sk::Object

namespace sk::Object
{
	typedef uint64_t hash_t;

	GENERATE_CLASS( iComponent, cSceneItem ), public cShared_from_this< iComponent >, public Event::cEventListener, public iSerializable
	{
		CREATE_CLASS_IDENTIFIERS( iComponent, runtime_class_iComponent )

		friend class cObject;
		friend class sk::cSceneManager;
	protected:
		iComponent()
		{
			m_transform_ = sk::MakeShared< cTransform >();
		}
	sk_public:

		explicit iComponent( cSerializedObject& _object );
		iComponent( iComponent const& ) = delete;

		~iComponent() override;

		// TODO: Make them protected
		// Event bases
		virtual void update      (){}
		virtual void render      (){}
		virtual void enabled     (){}
		virtual void disabled    (){}
		virtual void debug_render(){}

		[[ nodiscard ]]
		bool         GetEnabled() const { return m_enabled_; }
		virtual void SetEnabled( bool _is_enabled ) = 0;

		[[ nodiscard ]]
		bool         GetIsInternal() const { return m_internal_; }
		bool         GetIsRoot    () const { return m_is_root_; }

		virtual void PostEvent( uint16_t _event ) = 0;

		[[ nodiscard ]]
		auto& GetPosition()       { return m_transform_->GetLocalPosition(); }
		[[ nodiscard ]]
		auto& GetPosition() const { return m_transform_->GetLocalPosition(); }
		void  SetPosition( const cVector3f& _position ){ m_transform_->SetLocalPosition( _position ); }

		[[ nodiscard ]]
		auto& GetRotation()       { return m_transform_->GetLocalRotation(); }
		[[ nodiscard ]]
		auto& GetRotation() const { return m_transform_->GetLocalRotation(); }
		void  SetRotation( const cVector3f& _rotation ){ m_transform_->SetLocalRotation( _rotation ); }

		[[ nodiscard ]]
		auto& GetScale()       { return m_transform_->GetLocalScale(); }
		[[ nodiscard ]]
		auto& GetScale() const { return m_transform_->GetLocalScale(); }
		void  SetScale( const cVector3f& _scale ){ m_transform_->SetLocalScale( _scale ); }

		[[ nodiscard ]]
		auto& GetTransform() const { return *m_transform_; }
		
		[[ nodiscard ]]
		auto& GetSharedTransform() const { return m_transform_; }

		auto& GetChildren() const { return m_children_; }

		void SetParent( const cShared_ptr< iComponent >& _component );

		auto Serialize() -> cSerializedObject override;

	sk_protected:
		virtual void setEnabled( const bool _is_enabled ){ m_enabled_ = _is_enabled; }
		virtual void registerEvents() = 0;

		void SetObject( const cWeak_Ptr< cObject >& _parent_object );
		
		cShared_ptr< cTransform > m_transform_;
		cWeak_Ptr< iComponent >   m_parent_    = nullptr;
		cWeak_Ptr< cObject >      m_object_    = nullptr;
		// TODO: Have the UUID be able to be loaded from a scene file in the future.

		// TODO: Make the children into an unordered map

	sk_private: // TODO: Move parts to cpp, find way to make actual constexpr

		void registerRecursive();
		void enableRecursive ();
		void disableRecursive();

		std::vector< cShared_ptr< iComponent > > m_children_ = { }; // TODO: Add get children function
		bool  m_enabled_  = true;
		// Internal will hide it from the editor.
		bool  m_internal_ = false;
		bool  m_is_root_  = false;
	};

	// TODO: Check if type is necessary
	template< class Ty, uint16_t Events >
	class cComponent : public iComponent
	{
		friend class sk::cSceneManager;
#define HAS_EVENT( Func, Val ) if constexpr( !std::is_same_v< decltype( &Ty::Func ), decltype( &iComponent::Func ) > ) events |= (Val)
		constexpr static uint16_t detect_events()
		{
			uint16_t events = kNone;

			HAS_EVENT( update,       kUpdate      );
			HAS_EVENT( render,       kRender      );
			HAS_EVENT( enabled,      kEnabled     );
			HAS_EVENT( disabled,     kDisabled    );
			HAS_EVENT( debug_render, kDebugRender );

			return events;
		} // detectEvents
#undef HAS_EVENT
	protected:
		static constexpr uint16_t kEventMask = detect_events() & Events;

		cComponent() = default;
		explicit cComponent( cSerializedObject& _object ) : iComponent( _object ){}
	public:

		void SetEnabled( const bool _is_enabled ) final { setEnabled( _is_enabled ); _is_enabled ? postEvent< kEnabled >() : postEvent< kDisabled >(); }
		
		// Runtime postEvent
		void PostEvent( const uint16_t _event ) override
		{
			switch( _event )
			{
			case kEnabled:     postEvent< kEnabled     >(); break;
			case kDisabled:    postEvent< kDisabled    >(); break;
			case kUpdate:      postEvent< kUpdate      >(); break;
			case kRender:      postEvent< kRender      >(); break;
			case kDebugRender: postEvent< kDebugRender >(); break;
			default: break;
			}
		} // postEvent

		// Compile time postEvent
		template< uint16_t Event >
		constexpr void postEvent() // TODO: Add some sort of event input. Maybe Args?
		{
			constexpr auto mask = Event & kEventMask;
			// Constexpr "switch" as we only expect a single event at a time.
			if constexpr( mask == kUpdate      ) update      ();
			if constexpr( mask == kRender      ) render      ();
			if constexpr( mask == kEnabled     ) enabled     ();
			if constexpr( mask == kDisabled    ) disabled    ();
			if constexpr( mask == kDebugRender ) debug_render();
		} // postEvent

	private:
		void _updateInternal()
		{
			if constexpr( kEventMask & kUpdate )
			{
				update();
			}
			
			if( m_transform_->IsDirty() )
				m_transform_->Update();
		}
#ifdef SKAPE_EDITOR_AVAILABLE
		void _editorUpdateInternal()
		{
			if( m_transform_->IsDirty() )
				m_transform_->Update();
		}
#endif // SKAPE_EDITOR_AVAILABLE

		void registerEvents() final
		{
			RegisterListener( kUpdate, &cComponent::_updateInternal );
#ifdef SKAPE_EDITOR_AVAILABLE
			RegisterListener( kEditorUpdate, &cComponent::_editorUpdateInternal );
#endif // SKAPE_EDITOR_AVAILABLE
			if constexpr( kEventMask & kRender      ) RegisterListener( kRender,      &Ty::render       );
			if constexpr( kEventMask & kDebugRender ) RegisterListener( kDebugRender, &Ty::debug_render );
		} // register_events
	};

} // sk::Object

namespace sk::Object::Component
{
	using class_type = iComponent;
} // sk::Object::Component
SK_DECLARE_CLASS( sk::Object::Component )

#define COMPONENT_PARENT_CLASS( ComponentName, ... ) sk::Object::iComponent
#define COMPONENT_PARENT_VALIDATOR( ComponentName, ... ) std::is_base_of< sk::Object::iComponent, __VA_ARGS__ >
#define COMPONENT_PARENT_CREATOR_2( ComponentName, ... ) AFTER_FIRST( __VA_ARGS__ )
#define COMPONENT_PARENT_CREATOR_1( ComponentName, ... ) sk::Object::cComponent< M_CLASS( ComponentName ), sk::Object::kAll >
#define COMPONENT_PARENT_CREATOR( ComponentName, ... ) CONCAT( COMPONENT_PARENT_CREATOR_, VARGS( __VA_ARGS__ ) ) ( ComponentName, __VA_ARGS__ )
#define SK_COMPONENT_CLASS( ComponentName, ... ) QW_RESTRICTED_CLASS( ComponentName, COMPONENT_PARENT_CLASS, COMPONENT_PARENT_CREATOR, COMPONENT_PARENT_VALIDATOR, EMPTY __VA_OPT__( , __VA_ARGS__ ) )
