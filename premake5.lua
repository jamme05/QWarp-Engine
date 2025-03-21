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
    platforms { get_platform() }
    startproject "Startup"

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
        "NDEBUG", "RELEASE"
    }
    optimize "On"

filter { "configurations:Final" }
    defines {
        "NDEBUG",
        "FINAL",
        "QW_TRACKER_DISABLED"
    }
    optimize "On"

project "Framework"
    kind "StaticLib"
    location "build/framework"
    language "C++"
    targetdir "bin/Framework"

    files { "src/framework/**.hpp", "src/framework/**.cpp", "src/framework/**.h" }

    links { "QWarp", "Core" }
    
    includedirs { "src/engine", "src/framework", platform_src(), "external/fastgltf/include", "src/includes" }

project "QWarp"
    kind "StaticLib"
    location "build/engine"
    language "C++"
    targetdir "bin/QWarp" 

    links { "Shaders", "fastgltf", "lodepng", "Core" }

    files { "src/engine/**.hpp", "src/engine/**.cpp", "src/engine/**.h" }

    includedirs { "src/engine", platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }

project "Core"
    kind "StaticLib"
    location "build/core"
    language "C++"
    targetdir "bin/Core" 

    links { "Shaders", "fastgltf", "lodepng" }

    files( get_source_paths() )

    includedirs { platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }


project "Startup"
    kind "WindowedApp"
    location "build/startup"
    language "C++"
    targetdir "bin"

    links { "QWarp", "Framework", "Core" }

    files { "src/startup/main.cpp" }

    includedirs { "src/engine", "src/framework", platform_src(), "external/fastgltf/include", "external/lodepng", "src/includes" }

project "Visalizers"
    kind "None"
    location "build/visualizers"

project "Shaders"
    kind "StaticLib"
    location "build/shaders"
    language "C++"
    targetdir "bin/Shaders"

    files( shader_src() )

print( "Building fastgltf files...\n" )
os.execute( "cmake ./external/fastgltf" )
print( "\nBuild done." )

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