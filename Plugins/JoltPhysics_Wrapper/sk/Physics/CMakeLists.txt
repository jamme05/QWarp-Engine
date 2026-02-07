
target_sources(JoltPhysics_Wrapper
	PRIVATE
    Physics_Manager.cpp

  PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
      Physics_Manager.cpp
)

add_subdirectory(Components)
