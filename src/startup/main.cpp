/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#include "cApp.h"

#include <Types/types.h>
#include <Memory/Tracker/cTracker.h>

namespace qw
{
	static_assert( registry::type_registry< 0 >::valid, "No types registered." );
	constexpr static auto types = registry::type_registry< unique_id() - 1 >::registered;
	const unordered_map< type_hash, const sType_Info* > type_map = { types.begin(), types.end() };
} // qw::

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
