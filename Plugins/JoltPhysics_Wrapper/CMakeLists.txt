SkapePlugin(JoltPhysics_Wrapper)

target_compile_definitions(JoltPhysics_Wrapper PUBLIC SK_PHYSICS_JOLT)

# We want to use 32 bits.
set(OBJECT_LAYER_BITS 32)

add_subdirectory(JoltPhysics/Build EXCLUDE_FROM_ALL)

target_link_libraries(JoltPhysics_Wrapper
	PUBLIC
		Jolt::Jolt 
)

target_sources(JoltPhysics_Wrapper
	PUBLIC
    FILE_SET joltPhysicsIncludes
		TYPE HEADERS
    BASE_DIRS
      ./
)

add_subdirectory(sk)