
local plugins_config = "./Settings/plugins.json"
local plugins_dir    = "./Plugins/"
local plugins        = {}

function DefaultModuleFiles( basepath, types )
    local files = {}
    for _, t in pairs( types ) do
        table.insert( files, basepath .. t )
    end
    return files
end

function Default_Plugin_Setup( plugin_name )
    return function()
    project( plugin_name )
        kind( plugin_kind )
        location( "Build/Plugin/" .. plugin_name )
        language "C++"
        targetdir( "bin/Plugin/" .. plugin_name )
    
        files { DefaultModuleFiles( "Modules/" .. plugin_name .. "/src/", { ".hpp", ".h", ".cpp" } ) }
        
        includedirs { "src/engine" }
    end
end

function Load_Module( plugin_path )
    local plugin_dir       = modules_dir .. partial_plugin.Dir
    local plugin_file      = io.readfile( plugin_path .. "/plugin.json" )
    local plugin_json, err = json.decode( module_file )
    if( err ~= nil ) then
        print( "Warning: Module Path mismatch... Unable to find module with Id: " .. partial_plugin.Id .. " at directory " .. partial_plugin .. " Correcting TODO" )
        return
    end
    
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

function Supported_Platforms()
    return "Win64"
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

function CreateModules()
    ForeachModule( function( mod ) mod.Module_Project() end )
end