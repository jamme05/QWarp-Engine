/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cApp.h"

#include <Types/types.h>
#include "Memory/Tracker/cTracker.h"

int main( int, char** )
{
	qw::Memory::cTracker::initialize();
	auto& app = cApp::initialize();
	app.create();

	while( app.running() )
		app.run();

	app.destroy();

	cApp::deinitialize();

	qw::Memory::cTracker::deinitialize();

	return 0;
}
