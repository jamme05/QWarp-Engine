

#include "Physics_Manager.h"

#include <sk/Platform/Time.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <cstdarg>
#include <iostream>

#include "sk/Misc/Smart_Ptrs.h"

using namespace sk::Physics;

static void TraceImpl(const char *inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    ( void )vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
{
    // Print to the TTY
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

    // Breakpoint
    return true;
};

#endif // JPH_ENABLE_ASSERTS

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide( JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2 ) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
									BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

									JPH::uint					GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

									JPH::BroadPhaseLayer			GetBroadPhaseLayer( JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char *			GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool				ShouldCollide( JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// An example contact listener
class MyContactListener : public JPH::ContactListener
{
public:
	// See: ContactListener
	JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
	{
		std::cout << "Contact validate callback" << std::endl;

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	void			OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
	{
		std::cout << "A contact was added" << std::endl;
	}

	void			OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
	{
		std::cout << "A contact was persisted" << std::endl;
	}

	void			OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
	{
		std::cout << "A contact was removed" << std::endl;
	}
};

namespace
{
	BPLayerInterfaceImpl              broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broad_phase_layer_filter;
	ObjectLayerPairFilterImpl         object_layer_pair_filter;
} // ::

cPhysics_Manager::cPhysics_Manager( const uint8_t _threads )
{
    JPH::RegisterDefaultAllocator();

    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS( JPH::AssertFailed = AssertFailedImpl );

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

	m_job_system_ = std::make_unique< JPH::JobSystemThreadPool >( JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, _threads );

	m_physics_system_ = std::make_unique< JPH::PhysicsSystem >();
	auto& physics_system = *m_physics_system_;
	physics_system.Init(
		65536, 0, 65536, 10240,
		broad_phase_layer_interface, object_vs_broad_phase_layer_filter, object_layer_pair_filter
	);

	physics_system.SetBodyActivationListener( this );

	MyContactListener contact_listener;
	physics_system.SetContactListener( &contact_listener );
}

cPhysics_Manager::~cPhysics_Manager()
{
	m_physics_system_.reset();

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void cPhysics_Manager::Update()
{
    static JPH::TempAllocatorImpl temp_allocator( 10llu * 1024llu * 1024llu );

	auto& physics_system = *m_physics_system_;

	if( !m_queued_body_ids_.empty() && sk::Time::Frame % 120 )
	{
		JPH::BodyInterface& body_interface = physics_system.GetBodyInterface();
		const auto state = body_interface.AddBodiesPrepare( m_queued_body_ids_.data(), m_queued_body_ids_.size() );
		body_interface.AddBodiesFinalize( m_queued_body_ids_.data(), m_queued_body_ids_.size(), state, JPH::EActivation::DontActivate );

		physics_system.OptimizeBroadPhase();
		m_queued_body_ids_.clear();
	}

	physics_system.GetNarrowPhaseQuery().CastRay();
	physics_system.GetBroadPhaseQuery().CastRay();
	physics_system.Update( sk::Time::Delta, 1, &temp_allocator, m_job_system_.get() );
}

void cPhysics_Manager::AddLayer( const cStringID& _name, uint8_t _layer )
{

}

void cPhysics_Manager::RemoveLayer( uint8_t _layer )
{

}

void cPhysics_Manager::OnBodyActivated( const JPH::BodyID& inBodyID, uint64_t inBodyUserData )
{
	auto& weak = reinterpret_cast< cWeak_Ptr< Components::cShapeComponent >& >( inBodyUserData );
}

void cPhysics_Manager::OnBodyDeactivated( const JPH::BodyID& inBodyID, uint64_t inBodyUserData )
{
	auto& weak = reinterpret_cast< cWeak_Ptr< Components::cShapeComponent >& >( inBodyUserData );
}

JPH::ValidateResult cPhysics_Manager::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
	JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
{
	return ContactListener::OnContactValidate( inBody1, inBody2, inBaseOffset, inCollisionResult );
}

void cPhysics_Manager::OnContactAdded( const JPH::Body& inBody1, const JPH::Body& inBody2,
	const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings )
{

}

void cPhysics_Manager::OnContactPersisted( const JPH::Body& inBody1, const JPH::Body& inBody2,
	const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings )
{

}

void cPhysics_Manager::OnContactRemoved( const JPH::SubShapeIDPair& inSubShapePair )
{

}

bool cPhysics_Manager::ShouldCollide( JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2 ) const
{

}

JPH::uint cPhysics_Manager::GetNumBroadPhaseLayers() const
{

}

JPH::BroadPhaseLayer cPhysics_Manager::GetBroadPhaseLayer( JPH::ObjectLayer inLayer ) const
{

}

const char* cPhysics_Manager::GetBroadPhaseLayerName( JPH::BroadPhaseLayer inLayer ) const
{

}

bool cPhysics_Manager::ShouldCollide( JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2 ) const
{

}

void cPhysics_Manager::addBody( JPH::Body* _body )
{

}

void cPhysics_Manager::removeBody( JPH::BodyID _body_id )
{

}
