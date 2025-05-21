
-- Maybe move the fallback into its own directory?
local src_dir  = "../src/base/"

print( "WARNIG: QWarp has loaded the fallback build tool. In this environment compiling won't work." )

function Setup_Workspace()
    cppdialect "c++20"
    defines { "QW_CPP20" }
end

function get_core_id()
    return -1
end

function platform_src()
    return src_dir
end

function get_platforms()
    return "Invalid"
end

function get_source_paths()
    return src_dir .. "**"
end

function shader_src()
    return src_dir .. "Fallback_Shaders/**"
end