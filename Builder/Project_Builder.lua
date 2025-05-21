premake.override( path, "getDefaultSeparator", function( base )
	if( os.ishost( "windows" ) ) then
		return "\\"
	else
		return "/"
	end
end )

plugin_kind = "SharedLib" -- When making a game build use StaticLib.

include( "./utils/Module_Manager.lua" )

-- Return to root dir.
os.chdir( "../" )

Load_Modules()

-- TODO: Move to external file.
function CMakeBuilder( dependency )
    local dependency_path = "./Build/" .. dependency .. "/CMAKE"
    local return_path   = "../../../../"

    os.mkdir( dependency_path )
    os.chdir( dependency_path )
    print( "Building " .. dependency .. " files..." )
    os.execute( "cmake " .. return_path .. dependency )
    print( "Build done.\n" )
    os.chdir( return_path )
end

local root_build_dir = "./Build/Project/"

workspace "QWarp Playground"
    configurations { "Debug", "Release", "Final" }
    platforms { Get_Supported_Platforms() } -- Could allow me to use platforms as settings?
    startproject "Startup"

    defines {
        "LODEPNG_NO_COMPILE_ALLOCATORS",
    }

    files { "visualizers/*.natvis" }

    Setup_Workspace()

filter { "configurations:Debug" }
    defines {
        "DEBUG"
    }
    symbols "On"
    optimize "Off"

filter { "configurations:Release" }
    defines {
        "NDEBUG", "RELEASE", "QW_TRACKER_DISABLED"
    }
    symbols "On"
    optimize "Debug"

filter { "configurations:Final" }
    defines {
        "NDEBUG",
        "FINAL",
        "QW_TRACKER_DISABLED"
    }
    optimize "Full"

group "Game"

project "Framework"
    kind "StaticLib"
    location( root_build_dir .. "framework" )
    language "C++"
    targetdir "bin/Framework"

    files { "src/framework/**.hpp", "src/framework/**.cpp", "src/framework/**.h" }

    links { "Engine" }
    
    includedirs { "src/engine", "src/framework", "external/fastgltf/include", "src/base" }

project "Startup"
    kind "WindowedApp"
    location( root_build_dir .. "startup" )
    language "C++"
    targetdir "game/bin"

    links { "Engine", "Framework" }

    files { "src/startup/main.cpp" }

    includedirs { "src/engine", "src/framework", "external/fastgltf/include", "external/lodepng" }

group "QWarp"

project "Engine"
    kind "StaticLib"
    location( root_build_dir .. "engine" )
    language "C++"
    targetdir "bin/Engine" 

    links { "fastgltf", "lodepng", Get_Module_Links() }

    files { "src/engine/**.hpp", "src/engine/**.cpp", "src/engine/**.h" }

    includedirs { "src/engine", "external/fastgltf/include", "external/lodepng" }

group "Modules"

CreateModules()

group "Dependencies"

CMakeBuilder( "External/fastgltf" )
project "fastgltf"
    kind "StaticLib"
    location "Build/External/fastgltf"
    language "C++"
    targetdir "bin/external"

    includedirs { "external/fastgltf/include", "external/fastgltf/deps/simdjson/" }

    files { "external/fastgltf/src/**.cpp", "external/fastgltf/src/**.hpp", "external/fastgltf/deps/**.cpp", "external/fastgltf/deps/**.h" }
    
project "lodepng"
    kind "StaticLib"
    location "Build/External/lodepng"
    language "C++"
    targetdir "bin/external"
    
    includedirs { "external/lodepng" }
    
    files { "external/lodepng/*.cpp", "external/lodepng/*.h" }
    removefiles { "external/lodepng/lodepng_benchmark.*", "external/lodepng/lodepng_unittest.cpp" }