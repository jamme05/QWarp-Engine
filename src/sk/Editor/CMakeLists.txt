
# TODO: Only add to target if it's gonna build the Editor runtime.

list(FIND SKAPE_RUNTIMES_BUILD "Editor" EditorIndex)
if(EditorIndex GREATER -1)
    target_sources(SkapeEngine
        PRIVATE
            Editor.cpp

        PUBLIC
        FILE_SET engineIncludes
        TYPE HEADERS
        FILES
            Editor.h
    )

    add_subdirectory(Commands)
    add_subdirectory(Components)
    add_subdirectory(Managers)
    add_subdirectory(Tabs)
    add_subdirectory(Utils)
endif()
