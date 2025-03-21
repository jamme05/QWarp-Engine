/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once
#include <vector>

#include "Math/cTransform.h"
#include "Misc/Smart_ptrs.h"
#include "Components/iComponent.h"

namespace qw
{
	class cScene;
} // qw::

namespace qw::Object
{
// TODO: Check if class shit is needed? Like with Assets and Components.
	class iObject : public sShared_from_this< iObject >
	{
	public:
		         iObject( std::string _name ) : m_name ( std::move( _name ) ){ }
		virtual ~iObject( void ) = default;

		template< class Ty, class... Args >
		std::enable_if_t< std::is_base_of_v< iComponent, Ty >, cShared_ptr< Ty > >
		addComponent( Args... _args )
		{
			auto ptr = qw::make_shared< Ty >( _args... );

			ptr->m_object = get_shared_this();
			ptr->setParent( get_shared_this() );

			m_components.insert( { Ty::getClassType(), ptr } );
			return ptr;
		} // addComponent

		// TODO: Add an event system like the components have.
		virtual void render( void )
		{
			// TODO: Add actual event vector or something.
			for( auto& component : m_components )
			{
				component.second->postEvent( kRender );
				component.second->postEvent( kDebugRender );
			}
		}

		virtual void update( void )
		{
			for( auto& component : m_components )
				component.second->postEvent( kUpdate );
		}

		auto& getPosition ( void )       { return m_root->getPosition(); }
		auto& getPosition ( void ) const { return m_root->getPosition(); }

		auto& getRotation ( void )       { return m_root->getRotation(); }
		auto& getRotation ( void ) const { return m_root->getRotation(); }

		auto& getScale    ( void )       { return m_root->getScale(); }
		auto& getScale    ( void ) const { return m_root->getScale(); }

		auto& getTransform( void )       { return m_root->getTransform(); }
		auto& getTransform( void ) const { return m_root->getTransform(); }

		auto& getName( void ) const { return m_name; }

	protected:
		cShared_ptr< iComponent > m_root;

	private:
		// TODO: Move types to typedefs
		std::vector            < cShared_ptr< iObject > >                      m_children   = { };
		std::unordered_multimap< hash< uint64_t >, cShared_ptr< iComponent > > m_components = { };

		std::string m_name;

		cScene* m_parent_scene = nullptr;

		friend class cScene;

	};
} // qw::Scene::

