/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <string>

#include "Math/cMatrix4x4.h"
#include "Math/cTransform.h"
#include "Misc/Hashing.h"
#include "Misc/Smart_ptrs.h"
#include "Scene/Managers/cEventManager.h"

namespace qw::Object
{
	class iObject;
} // qw::Object

namespace qw::Object
{
	typedef uint64_t hash_t;

	class iComponent
	{
	protected:
		iComponent( void ) = default;
	public:

		iComponent( iComponent const& ) = delete;

		virtual ~iComponent( void ) = default;

		virtual       hash_t       getType    ( void ) const = 0;
		virtual const std::string& getTypeName( void ) const = 0;

		// Event bases
		virtual void update      ( void ){}
		virtual void render      ( void ){}
		virtual void enabled     ( void ){}
		virtual void disabled    ( void ){}
		virtual void debug_render( void ){}

		virtual void setEnabled( const bool _is_enabled ){ m_enabled = _is_enabled; }

		virtual void postEvent( uint16_t _event ) = 0;

		auto& getPosition ( void )       { return m_transform.getPosition(); }
		auto& getPosition ( void ) const { return m_transform.getPosition(); }

		auto& getRotation ( void )       { return m_transform.getRotation(); }
		auto& getRotation ( void ) const { return m_transform.getRotation(); }

		auto& getScale    ( void )       { return m_transform.getScale(); }
		auto& getScale    ( void ) const { return m_transform.getScale(); }

		auto& getTransform( void )       { return m_transform; }
		auto& getTransform( void ) const { return m_transform; }

		void setParent( const cShared_ptr< iComponent >& _component ){ m_parent = _component; m_transform.setParent( _component->getTransform() ); }

	protected:
		cTransform                               m_transform = {};
		cWeak_Ptr< iComponent >                  m_parent    = nullptr;
		cWeak_Ptr< iObject >                     m_object    = nullptr;

	private: // TODO: Move parts to cpp, find way to make actual constexpr
		bool               m_enabled = true;
		friend class iObject;
	};

	namespace Components
	{
#define HAS_EVENT( Func ) !std::is_same_v< decltype( &Child::Func ), decltype( &iComponent::Func ) >

        template< class Child = iComponent >
        constexpr std::enable_if_t< std::is_base_of_v< iComponent, Child >, uint16_t >
        detect_events( void )
        {
        	uint16_t events = kNone;

        	if constexpr( HAS_EVENT( update ) )
        		events |= kUpdate;

        	if constexpr( HAS_EVENT( render ) )
        		events |= kRender;

        	if constexpr( HAS_EVENT( enabled ) )
        		events |= kEnabled;

        	if constexpr( HAS_EVENT( disabled ) )
        		events |= kDisabled;

        	if constexpr( HAS_EVENT( debug_render ) )
        		events |= kDebugRender;

        	return events;
        } // detectEvents

#undef HAS_EVENT
	} // Components::

	// TODO: Check if type is necessary
	template< class Ty, const char* Name, hash_t Hash, uint16_t Events >
	class cComponent : public iComponent, public sShared_from_this< Ty >
	{
	protected:
		// Used to disable events not finished yet overriden.
		// Registers all overriden events as callable events.
		cComponent( void )
		: iComponent()
		{
			register_events();
		} // cComponent

	public:
		hash_t             getType    ( void ) const final { return type_hash; }
		const std::string& getTypeName( void ) const final { return type_name; }

		// Static variant of getType
		static constexpr auto  getClassType( void ){ return type_hash; }
		// Static variant of getTypeName
		static constexpr auto& getClassName( void ){ return type_name; }

		void setEnabled( const bool _is_enabled ) override { iComponent::setEnabled( _is_enabled ); _is_enabled ? postEvent< kEnabled >() : postEvent< kDisabled >(); }

		// Runtime postEvent
		void postEvent( const uint16_t _event ) override
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
		constexpr void postEvent( void ) // TODO: Add some sort of event input. Maybe Args?
		{
			constexpr auto mask = Events & Event;
			// Constexpr "switch" as we only expect a single event at a time.
			if constexpr( mask == kUpdate      ) update      ();
			if constexpr( mask == kRender      ) render      ();
			if constexpr( mask == kEnabled     ) enabled     ();
			if constexpr( mask == kDisabled    ) disabled    ();
			if constexpr( mask == kDebugRender ) debug_render();
		} // postEvent

	private:
		void register_events( void )
		{
			if constexpr( Events & kUpdate      ) cEventManager::get().registerLister( kUpdate,      std::bind( &Ty::update,       static_cast< Ty* >( this ) ) );
			if constexpr( Events & kRender      ) cEventManager::get().registerLister( kRender,      std::bind( &Ty::render,       static_cast< Ty* >( this ) ) );
			if constexpr( Events & kEnabled     ) cEventManager::get().registerLister( kEnabled,     std::bind( &Ty::enabled,      static_cast< Ty* >( this ) ) );
			if constexpr( Events & kDisabled    ) cEventManager::get().registerLister( kDisabled,    std::bind( &Ty::disabled,     static_cast< Ty* >( this ) ) );
			if constexpr( Events & kDebugRender ) cEventManager::get().registerLister( kDebugRender, std::bind( &Ty::debug_render, static_cast< Ty* >( this ) ) );
		} // register_events

	protected:
		std::vector< cShared_ptr< iComponent > > m_children = { };

		static constexpr    hash_t      type_hash = Hash;
		static const inline std::string type_name = Name;
	};

} // qw::Object

#define QW_COMPONENT_CLASS( ComponentName ) \
class M_CLASS( ComponentName ); \
namespace ComponentName { \
static constexpr char     Component_Name[] = #ComponentName; \
static constexpr hash_t   Type   = Hashing::fnv1a_32( Component_Name ); \
typedef qw::cShared_ptr< M_CLASS( ComponentName ) > Shared_t; \
} \
class M_CLASS( ComponentName ) : public qw::Object::cComponent< M_CLASS( ComponentName ), ComponentName::Component_Name, ComponentName::Type, qw::Object::kAll >