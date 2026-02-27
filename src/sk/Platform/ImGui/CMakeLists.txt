

target_sources(SkapeEngine
    PRIVATE
        ImGuiHelper.cpp

    PUBLIC
    FILE_SET engineIncludes
    TYPE HEADERS
    FILES
        ImGuiHelper.h
)