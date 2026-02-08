
target_sources(SkapeEngine
  PRIVATE
    Scene.cpp
    Object.cpp
    Layer.cpp

  PUBLIC
    FILE_SET engineIncludes
    TYPE HEADERS
    FILES
      Object.h
      Scene.h
      Layer.h
)

add_subdirectory(Components)
add_subdirectory(Managers)
add_subdirectory(Objects)