
include "../build_target"

local core_dir      = "../src/core/"
local platform_file = "/platform_id"

function file_exists( file )
    local f = io.open( file )
    if f then f:close() end
    return f ~= nil
end

function check_platform_id( path )
    local p_f = path .. platform_file
    if file_exists( p_f ) then
        local c = io.readfile( p_f )
        if get_build_target() == tonumber( c ) then
            include( path .. "/platform_setup" )
        end
    end
end

function check_for_platform()
    local available_platforms = {}
    table.insert( available_platforms, "ps5" )
    table.insert( available_platforms, "win64" )

    for _, p in ipairs( available_platforms ) do
        check_platform_id( core_dir .. p )
    end
end

check_for_platform()
