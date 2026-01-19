local jolt_dir = "JoltPhysics"
local jolt_include_dir = jolt_dir
local jolt_asset_dir = path.join( jolt_dir, "Assets" )
local jolt_build_dir = "Build/Plugins/JoltPhysics/JoltPhysics/cmake/Release/"
local jolt_build_settings = "-DOBJECT_LAYER_BITS=32"

plugin = {
    Name = "Jolt Physics",
    Setup_Workspace = function( plugin )
        filter( plugin.Filter )
        defines( { "QW_PHYSICS_JOLT" } )
    end,

    Init = function( plugin_dir )
        CMakeBuilder( path.join( plugin_dir, jolt_dir ), "Build", "Release", jolt_build_settings )

        CopyFolder( path.join( plugin_dir, jolt_asset_dir ), path.join( jolt_build_dir, "Assets" ) )
    end,

    IncludeLibs = function( plugin_dir )
        return {
            path.join( jolt_build_dir, "Jolt.lib" ),
        }
    end,

    -- TODO: Add cache.
    IncludeDirs = function( plugin_dir )
        return { path.join( "Plugins", plugin_dir, jolt_include_dir ) }
    end,
}
plugin.Plugin_Project = Plugin_Setup( "Jolt Physics",
    plugin.IncludeLibs,
    plugin.IncludeDirs
)
