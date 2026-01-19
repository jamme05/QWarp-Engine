

#pragma once

#include <Misc/Singleton.h>

namespace sk::Physics
{
    class cPhysics_Manager : public cSingleton< cPhysics_Manager >
    {
    public:
        cPhysics_Manager( uint8_t _threads = 4 );
        ~cPhysics_Manager();

    private:
        
    };
} // sk::Physics::
