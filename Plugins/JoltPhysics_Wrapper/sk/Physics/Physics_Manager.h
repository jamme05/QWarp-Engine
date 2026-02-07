

#pragma once

#include <memory>
#include <sk/Misc/Singleton.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace JPH
{
    class PhysicsSystem;
    class JobSystemThreadPool;
} // JPH::

namespace sk::Physics
{
    namespace Components
    {
        class cShapeComponent;
        class cJoltPhysicsInternalComponent;
    } // sk::Physics::Components::

    class cPhysics_Manager : public cSingleton< cPhysics_Manager >
    {
        friend class sk::Physics::Components::cJoltPhysicsInternalComponent;
        friend class sk::Physics::Components::cShapeComponent;
    public:
        explicit cPhysics_Manager( uint8_t _threads = 4 );
        ~cPhysics_Manager() final;

        void Update();

    private:
        struct sBody
        {

        };
        std::vector< JPH::BodyID > m_queued_body_ids_;
        std::vector< sBody >       m_queued_bodies_;

        std::unique_ptr< JPH::PhysicsSystem       > m_physics_system_;
        std::unique_ptr< JPH::JobSystemThreadPool > m_job_system_;

    };
} // sk::Physics::
