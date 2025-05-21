
module = {
    Name = "Vulkan",
    Setup_Workspace = function()
        defines( { "QW_GRAPHICS_VULKAN" } )
    end,

    Module_Project = Default_Module_Setup( "Vulkan" )
}