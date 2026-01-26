
target_sources(JoltPhysics
	PRIVATE
    Physics_Manager.cpp

  PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
      Physics_Manager.cpp
)
