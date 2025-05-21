
module = {
    Name = "Windows",
    Setup_Workspace = function()
        cppdialect( "c++20" ) -- Add something like Maximum Cpp version/dialect?

        -- May remove QW_CPP20 later.
        defines( { "QW_TARGET_WIN64", "QW_CPP20" } )
    end,

    Module_Project = Default_Module_Setup( "Windows" )
}