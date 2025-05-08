/*
 *
 * COPYRIGHT William Ask S. Ness 2025
 *
 */

#pragma once

#include <Types/types.h>

#include "Misc/cSingleton.h"

namespace qw
{
	class cLightManager : public cSingleton< cLightManager >
	{
	public:
		cLightManager();
		~cLightManager();

		uint64_t registerLight(  );
	};
}
