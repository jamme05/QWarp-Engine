
target_sources(JoltPhysics_Wrapper
	PRIVATE
    BoxColliderComponent.cpp
    RigidBodyComponent.cpp
  PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
    BoxColliderComponent.h
    RigidBodyComponent.h
)
