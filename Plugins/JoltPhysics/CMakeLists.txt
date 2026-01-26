SkapePlugin(JoltPhysics)

target_compile_definitions(JoltPhysics PUBLIC SK_PHYSICS_JOLT)

add_subdirectory(JoltPhysics/Build)

target_sources(JoltPhysics
	PUBLIC
    FILE_SET joltPhysicsIncludes
		TYPE HEADERS
    BASE_DIRS
      ./
)

add_subdirectory(sk)