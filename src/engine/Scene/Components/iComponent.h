/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <string>
#include <agc/registerstructs.h>

#include "Math/cMatrix4x4.h"
#include "Math/cTransform.h"
#include "Misc/Hashing.h"
#include "Misc/Smart_ptrs.h"

namespace qw::Object {
	class iObject;
}

namespace qw::Object
{
	typedef uint64_t hash_t;

	namespace Components
	{
		enum eEvents : uint16_t // TODO: Create an event listener class which works for both objects and components
		{
			kNone     = 0x00,

			kUpdate   = 0x01,
			kRender   = 0x02,
			kUpdateRender = kUpdate | kRender,

			kEnabled  = 0x04,
			kDisabled = 0x10,
			kEnabledDisabled = kEnabled | kDisabled,

			kDebugRender = 0x20,
			kAllDebug    = kDebugRender, // TOOD: Add more debug events

			kAll    = 0xffff,
		};
	} // Components

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

		auto& getTransform( void )       { return m_transform; }
		auto& getTransform( void ) const { return m_transform; }

	protected:
		cTransform                               m_transform = {};
		iComponent*                              m_parent    = nullptr;
		iObject*                                 m_object    = nullptr;

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
		cComponent( void ) : iComponent(){ }

	public:
		hash_t             getType    ( void ) const final { return type_hash; }
		const std::string& getTypeName( void ) const final { return type_name; }

		// Static variant of getType
		static constexpr auto  getClassType( void ){ return type_hash; }
		// Static variant of getTypeName
		static constexpr auto& getClassName( void ){ return type_name; }

		void setEnabled( const bool _is_enabled ) override { iComponent::setEnabled( _is_enabled ); _is_enabled ? postEvent< Components::kEnabled >() : postEvent< Components::kDisabled >(); }

		// Runtime postEvent
		void postEvent( const uint16_t _event ) override
		{
			switch( _event )
			{
			case Components::kEnabled:     postEvent< Components::kEnabled     >(); break;
			case Components::kDisabled:    postEvent< Components::kDisabled    >(); break;
			case Components::kUpdate:      postEvent< Components::kUpdate      >(); break;
			case Components::kRender:      postEvent< Components::kRender      >(); break;
			case Components::kDebugRender: postEvent< Components::kDebugRender >(); break;
			default: break;
			}
		} // postEvent

		// Compile time postEvent
		template< uint16_t Event >
		constexpr void postEvent( void ) // TODO: Add some sort of event input. Maybe Args?
		{
			constexpr auto mask = Events & Event;
			// Constexpr "switch" as we only expect a single event at a time.
			if constexpr( mask == Components::kUpdate      ) update      ();
			if constexpr( mask == Components::kRender      ) render      ();
			if constexpr( mask == Components::kEnabled     ) enabled     ();
			if constexpr( mask == Components::kDisabled    ) disabled    ();
			if constexpr( mask == Components::kDebugRender ) debug_render();
		}


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
class M_CLASS( ComponentName ) : public qw::Object::cComponent< M_CLASS( ComponentName ), ComponentName::Component_Name, ComponentName::Type, qw::Object::Components::kAll >