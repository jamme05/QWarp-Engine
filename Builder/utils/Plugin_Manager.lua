
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

function Default_Plugin_Setup( name, path )
    project( name )
        kind( plugin_kind )
        location( "Build/Plugin/" .. name )
        language "C++"
        targetdir( "bin/Plugin/" .. name )
    
        files { DefaultModuleFiles( "Modules/" .. path .. "/src/", { ".hpp", ".h", ".cpp" } ) }
        
        includedirs { "src/engine" }
end

function Load_Module( plugin_path )
    local plugin_dir       = modules_dir .. partial_plugin.Dir
    local plugin_file      = io.readfile( plugin_path .. "/plugin.json" )
    local plugin_json, err = json.decode( module_file )
    if( err ~= nil ) then
        print( "Warning: Module Path mismatch... Unable to find module with Id: " .. partial_plugin.Id .. " at directory " .. partial_plugin .. " Correcting TODO" )
        return
    end
    
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

function ForeachPlugin( run )
    if modules == nil then
        return
    end

    for _, mod in pairs( modules ) do
        run( mod )
    end
end

function CreateModules()
    ForeachPlugin( function( p ) Default_Plugin_Setup( p.Name, p.Dir ) end )
end