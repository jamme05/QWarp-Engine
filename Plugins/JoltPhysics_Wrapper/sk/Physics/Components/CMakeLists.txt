
target_sources(JoltPhysics_Wrapper
  PRIVATE
		BoxColliderComponent.cpp
		RigidBodyComponent.cpp
		JoltPhysicsInternalComponent.cpp
		ShapeComponent.cpp
  PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
		BoxColliderComponent.h
		RigidBodyComponent.h
		JoltPhysicsInternalComponent.h
		ShapeComponent.h
)
