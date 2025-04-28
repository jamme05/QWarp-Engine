/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "Math/cTransform.h"
#include "Misc/Smart_ptrs.h"
#include "Components/iComponent.h"

#include <ranges>
#include "qw_std/vector.h"
#include "qw_std/map.h"

#include "Components/cTransformComponent.h"


namespace qw
{
	class cScene;
} // qw::

namespace qw::Object
{
// TODO: Check if class shit is needed? Like with Assets and Components.
	class iObject : public Event::cEventListener, public cShared_from_this< iObject >
	{
	public:
		// TODO: Create templated constructor with root type + parameters
		explicit iObject( std::string _name )
		: m_root( qw::make_shared< Components::cTransformComponent >() )
		, m_name( std::move( _name ) )
	{
		} // iObject

		template< class Ty = iComponent, class... Args >
		explicit iObject( std::string _name, Args... _args )
		: m_root( qw::make_shared< Ty >( _args... ) )
		, m_name( std::move( _name ) )
	{
		} // iObject

		virtual ~iObject( void )
		{
			m_children.clear();
			m_components.clear();
			m_root = nullptr;
		}

		template< class Ty, class... Args >
		requires std::is_base_of_v< iComponent, Ty >
		cShared_ptr< Ty > addComponent( Args&&... _args )
		{
			auto ptr = qw::make_shared< Ty >( std::forward< Args >( _args )... );

			ptr->m_object = get_weak_this();
			ptr->setParent( m_root );

			m_components.insert( std::pair{ Ty::getClassType(), ptr } );

			return ptr;
		} // addComponent

		// TODO: Add an event system like the components have.
		virtual void render( void )
		{
			// TODO: Add actual event vector or something.
			for( auto& val : m_components | std::views::values )
			{
				val->postEvent( kRender );
				val->postEvent( kDebugRender );
			}
		}

		virtual void update( void )
		{
			for( auto& val : m_components | std::views::values )
				val->postEvent( kUpdate );
		}

		auto& getRoot( void )       { return m_root; }
		auto& getRoot( void ) const { return m_root; }

		auto& getPosition ( void )       { return m_root->getPosition(); }
		auto& getPosition ( void ) const { return m_root->getPosition(); }

		auto& getRotation ( void )       { return m_root->getRotation(); }
		auto& getRotation ( void ) const { return m_root->getRotation(); }

		auto& getScale    ( void )       { return m_root->getScale(); }
		auto& getScale    ( void ) const { return m_root->getScale(); }

		auto& getTransform( void )       { return m_root->getTransform(); }
		auto& getTransform( void ) const { return m_root->getTransform(); }

		auto& getName     ( void ) const { return m_name; }

	protected:
		cShared_ptr< iComponent > m_root;

	private:
		// TODO: Move types to typedefs
		vector            < cShared_ptr< iObject > >    m_children   = { };
		multimap< uint64_t, cShared_ptr< iComponent > > m_components = { };

		std::string m_name;

		cScene* m_parent_scene = nullptr;

		friend class cScene;

	};
} // qw::Scene::

