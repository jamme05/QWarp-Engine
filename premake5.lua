premake.override( path, "getDefaultSeparator", function( base )
	if( os.ishost( "windows" ) ) then
		return "\\"
	else
		return "/"
	end
end )

include "load_target"

workspace "QWarp Playground"
    configurations { "Debug", "Release", "Final" }
    platforms { get_platforms() } -- Could allow me to use platforms as settings?
    startproject "Startup"

    defines {
        "LODEPNG_NO_COMPILE_ALLOCATORS"
    }

    files { "visualizers/*.natvis" }

    SetupFilters()

filter { "platforms:x64" }
    defines {
        "QW_TARGET_WIN64",
        "QW_GRAPHICS_TBD",
    }
    architecture "x86_64"

filter { "configurations:Debug" }
    defines {
        "DEBUG"
    }
    symbols "On"

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
    location "build/framework"
    language "C++"
    targetdir "bin/Framework"

    files { "src/framework/**.hpp", "src/framework/**.cpp", "src/framework/**.h" }

    links { "Engine", "Core" }
    
    includedirs { "src/engine", "src/framework", platform_src(), "external/fastgltf/include", "src/includes" }

project "Startup"
    kind "WindowedApp"
    location "build/startup"
    language "C++"
    targetdir "game/bin"

    links { "Engine", "Framework", "Core" }

    files { "src/startup/main.cpp" }

    includedirs { "src/engine", "src/framework", platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }

group "QWarp"

project "Engine"
    kind "StaticLib"
    location "build/engine"
    language "C++"
    targetdir "bin/QWarp" 

    links { "Shaders", "fastgltf", "lodepng", "Core" }

    files { "src/engine/**.hpp", "src/engine/**.cpp", "src/engine/**.h" }

    includedirs { "src/engine", platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }


project "Includes"
    kind "None"
    location "build/includes"
    language "C++"

    files( "src/includes/**" )

    includedirs { platform_src(), "src/engine", "src/includes" }

project "Core"
    kind "StaticLib"
    location "build/core"
    language "C++"
    targetdir "bin/Core" 

    links { "Shaders", "fastgltf", "lodepng" }

    files( get_source_paths() )

    includedirs { platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }

project "Shaders"
    kind "StaticLib"
    location "build/shaders"
    language "C++"
    targetdir "bin/Shaders"

    files( shader_src() )

os.mkdir( "./build/fastgltf" )
os.chdir( "./build/fastgltf" )
print( "Building fastgltf files...\n" )
os.execute( "cmake ../../external/fastgltf" )
print( "\nBuild done." )
os.chdir( "../.." )

group "Dependencies"

project "fastgltf"
    kind "StaticLib"
    location "build/external/fastgltf"
    language "C++"
    targetdir "bin/external"

    includedirs { "external/fastgltf/include", "external/fastgltf/deps/simdjson/" }

    files { "external/fastgltf/src/**.cpp", "external/fastgltf/src/**.hpp", "external/fastgltf/deps/**.cpp", "external/fastgltf/deps/**.h" }
    
project "lodepng"
    kind "StaticLib"
    location "build/external/lodepng"
    language "C++"
    targetdir "bin/external"
    
    includedirs { "external/lodepng" }
    
    files { "external/lodepng/*.cpp", "external/lodepng/*.h" }
    removefiles { "external/lodepng/lodepng_benchmark.*", "external/lodepng/lodepng_unittest.cpp" }