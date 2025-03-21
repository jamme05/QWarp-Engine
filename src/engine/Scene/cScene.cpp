/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cScene.h"

namespace qw
{
	void cScene::force_render( void )
	{
		// TODO: Get rid of this

		for( auto& obj : m_objects )
		{
			obj->render();
		}
	} // render

	void cScene::force_update( void )
	{
		for( auto& obj : m_objects )
			obj->update();
	} // update
} // qw::
