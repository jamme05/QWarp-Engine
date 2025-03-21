/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include "cObject.h"
#include "Assets/iAsset.h"


namespace qw
{
	namespace Object::Components
	{
		class cCamera;
	} // Objects::Components

	QW_ASSET_CLASS( Scene )
	{
	public:
		explicit cScene( const std::string& _name ) : cAsset( _name ){ }

		// Creates an empty object, essentially an empty
		template< class Ty, class... Args >
		std::enable_if_t< std::is_base_of_v< Object::iObject, Ty >, cShared_ptr< Ty > >
		create_object( const std::string& _name, Args... _args )
		{
			auto shared = qw::make_shared< Ty >( _name, _args... );
			m_objects.emplace_back( shared );
			return shared;
		} // create_object

		void force_render( void );
		void force_update( void );

	private:
		// TODO: Replace this with a map.
		std::vector< cShared_ptr< Object::iObject > > m_objects = {};
	};

} // qw::