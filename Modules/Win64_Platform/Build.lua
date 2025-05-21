
local module = {
    Setup_Workspace = function()
        defines { "QW_TARGET_WIN64" }
    end

    Module_Project = function()
        kind "StaticLib"
        location( "Build/Module/Win64" )
        language "C++"
        targetdir "bin/Module/Win64"
    
        files { "Modules/Win64/**.hpp", "Modules/Win64/**.cpp", "Modules/Win64/**.h" }
        
        includedirs { "src/engine" }
    
    end
}