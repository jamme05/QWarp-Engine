
target_sources(JoltPhysics_Wrapper
  PRIVATE
	Physics_Manager.cpp
	PhysicsLayer.cpp

  PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
      Physics_Manager.h
	  PhysicsLayer.h
)

add_subdirectory(Components)
add_subdirectory(Utils)