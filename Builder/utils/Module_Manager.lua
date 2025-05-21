
local modules_config = "./Settings/modules.json"
local modules_dir    = "./Modules/"
local modules        = {}

function DefaultModuleFiles( basepath, types )
    local files = {}
    for _, t in pairs( types ) do
        table.insert( files, basepath .. t )
    end
    return files
end

function Default_Module_Setup( module_name )
    return function( module_dir )
    project( module_name )
        kind "StaticLib"
        location( "Build/Module/" .. module_name )
        language "C++"
        targetdir( "bin/Module/" .. module_name )
    
        files { DefaultModuleFiles( "Modules/" .. module_dir .. "/src/**", { ".hpp", ".h", ".cpp" } ) }
        
        includedirs { "src/engine" }
    end
end

function Load_Module( partial_module )
    local module_dir       = modules_dir .. partial_module.Dir
    local module_file      = io.readfile( module_dir .. "/module.json" )
    local module_json, err = json.decode( module_file )
    if( err ~= nil ) then
        print( "Warning: Module Path mismatch... Unable to find module with Id: " .. partial_module.Id .. " at directory " .. module_dir .. " Correcting TODO" )
        return
    end

    local build_file = module_json[ "BuildFile" ] or "Build.lua"


    include( module_dir .. "/" .. build_file )
    module.Raw = module_json
    module.Dir = partial_module.Dir
    table.insert( modules, module )
end

function ValidateModules()
end

function Load_Modules()
    local modules_file      = io.readfile( modules_config )
    local modules_json, err = json.decode( modules_file )
    if( err ~= nil ) then
        print( err )
        return
    end
    
    for _, mod in pairs( modules_json ) do
        Load_Module( mod )
    end


    -- Will throw error inside itself if invalid.
    ValidateModules()
end

function Get_Supported_Platforms()
    local platforms_parser = {}
    ForeachModule( function( mod )
        -- Only platform in case it's coming from a platform module.
        if mod.Raw.Type ~= "Platform" then
            return
        end
        for _, p in pairs( mod.Raw.Supports.Platforms ) do
            platforms_parser[ p ] = 1
        end
    end )
    local platforms = {}
    for p, _ in pairs( platforms_parser ) do
        table.insert( platforms, p )
    end
    return platforms
end

function ForeachModule( run )
    if modules == nil then
        return
    end

    for _, mod in pairs( modules ) do
        run( mod )
    end
end

function Setup_Workspace()
    ForeachModule( function( mod ) mod.Setup_Workspace() end )
end

function Get_Module_Links()
    local links = {}
    ForeachModule( function( mod )
        table.insert( links, mod.Name )
    end )
    return links;
end

function CreateModules()
    ForeachModule( function( mod ) mod.Module_Project( mod.Dir ) end )
end