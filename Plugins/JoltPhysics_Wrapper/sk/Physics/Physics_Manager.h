

#pragma once

#include <sk/Misc/Singleton.h>
#include <sk/Misc/StringID.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include <memory>



namespace JPH
{
    class Body;
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

    class cPhysics_Manager : public cSingleton< cPhysics_Manager >,
        public JPH::BodyActivationListener,
        public JPH::ObjectLayerPairFilter,
        public JPH::BroadPhaseLayerInterface,
        public JPH::ObjectVsBroadPhaseLayerFilter,
        public JPH::ContactListener
    {
        friend class sk::Physics::Components::cJoltPhysicsInternalComponent;
        friend class sk::Physics::Components::cShapeComponent;
    public:
        explicit cPhysics_Manager( uint8_t _threads = 4 );
        ~cPhysics_Manager() final;

        void Update();

        void AddLayer   ( const cStringID& _name, uint8_t _layer );
        void RemoveLayer( uint8_t _layer );

        const auto& GetPhysicsSystem() const { return *m_physics_system_; }

    protected:
        void OnBodyActivated  ( const JPH::BodyID& inBodyID, uint64_t inBodyUserData ) override;
        void OnBodyDeactivated( const JPH::BodyID& inBodyID, uint64_t inBodyUserData ) override;
        JPH::ValidateResult OnContactValidate( const JPH::Body& inBody1, const JPH::Body& inBody2,
            JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult ) override;
        void OnContactAdded( const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
            JPH::ContactSettings& ioSettings ) override;
        void OnContactPersisted( const JPH::Body& inBody1, const JPH::Body& inBody2,
            const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings ) override;
        void OnContactRemoved( const JPH::SubShapeIDPair& inSubShapePair ) override;
        bool ShouldCollide( JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2 ) const override;
        JPH::uint GetNumBroadPhaseLayers() const override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer( JPH::ObjectLayer inLayer ) const override;
        const char* GetBroadPhaseLayerName( JPH::BroadPhaseLayer inLayer ) const override;
        bool ShouldCollide( JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2 ) const override;
    private:
        void addBody   ( JPH::Body* _body );
        void removeBody( JPH::BodyID _body_id );

    private:
        struct sBody
        {
            JPH::Body* body;
        };
        std::vector< JPH::BodyID > m_queued_body_ids_;
        std::vector< sBody >       m_queued_bodies_;

        std::unique_ptr< JPH::PhysicsSystem       > m_physics_system_;
        std::unique_ptr< JPH::JobSystemThreadPool > m_job_system_;

    };
} // sk::Physics::
