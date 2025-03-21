/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once
#include <vector>

#include "Components/iComponent.h"
#include "Math/cTransform.h"
#include "Misc/Smart_ptrs.h"

namespace qw
{
	class cScene;
} // qw::

namespace qw::Object
{
// TODO: Check if class shit is needed? Like with Assets and Components.
	class iObject
	{
	public:
		iObject( std::string _name ) : m_name ( std::move( _name ) ){}

		template< class Ty, class... Args >
		std::enable_if_t< std::is_base_of_v< iComponent, Ty >, cShared_ptr< Ty > >
		addComponent( Args... _args )
		{
			auto ptr = qw::make_shared< Ty >( _args... );

			ptr->m_object = this;
			ptr->m_transform.setParent( m_transform );

			m_components.insert( { Ty::getClassType(), ptr } );
			return ptr;
		} // addComponent

		// TODO: Add an event system like the components have.
		virtual void render( void )
		{
			// TODO: Add actual event vector or something.
			for( auto& component : m_components )
			{
				component.second->postEvent( Components::kRender );
				component.second->postEvent( Components::kDebugRender );
			}
		}

		virtual void update( void )
		{
			for( auto& component : m_components )
				component.second->postEvent( Components::kUpdate );
		}

		auto& getTransform( void )       { return m_transform; }
		auto& getTransform( void ) const { return m_transform; }

		auto& getName( void ) const { return m_name; }

	protected:
		cTransform m_transform;

	private:
		// TODO: Move types to typedefs
		std::vector            < cShared_ptr< iObject > >                      m_children   = { };
		std::unordered_multimap< hash< uint64_t >, cShared_ptr< iComponent > > m_components = { };

		std::string m_name;

		cScene* m_parent_scene = nullptr;

		friend class cScene;

	};
} // qw::Scene::

