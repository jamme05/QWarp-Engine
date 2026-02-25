
target_sources(SkapeEngine
    PRIVATE
        Command.cpp

    PUBLIC
    FILE_SET engineIncludes
    TYPE HEADERS
    FILES
        Command.h
)