
target_sources(JoltPhysics_Wrapper
    PRIVATE
        RayCast.cpp

    PUBLIC
    FILE_SET joltPhysicsIncludes
    TYPE HEADERS
    FILES
        RayCast.h
)