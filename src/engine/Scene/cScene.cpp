/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cScene.h"

namespace qw
{
	void cScene::render( void )
	{
		// TODO: Get rid of this

		for( auto& obj : m_objects )
		{
			obj->render();
		}
	} // render

	void cScene::update( void )
	{
		for( auto& obj : m_objects )
			obj->update();
	} // update
} // qw::
