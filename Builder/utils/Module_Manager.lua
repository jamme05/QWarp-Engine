
local modules_config = "./Settings/modules.json"
local modules_dir    = "./Modules/"
local modules        = {}

function Load_Module( partial_module )
    local module_dir       = modules_dir .. partial_module.Dir
    local module_file      = io.readfile( module_dir .. "/module.json" )
    local module_json, err = json.decode( module_file )
    if( err ~= nil ) then
        print( "Warning: Module Path mismatch... Unable to find module with Id: " .. partial_module.Id .. " At directory " .. module_dir .. " Correcting TODO" )
        return
    end

    include( module_dir )

    print( module_file )
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