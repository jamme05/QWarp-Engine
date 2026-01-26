
target_sources(JoltPhysics
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
