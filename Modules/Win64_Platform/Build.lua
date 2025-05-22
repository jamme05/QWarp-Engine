
module = {
    Name = "Windows",
    Setup_Workspace = function()
        filter{ "platforms:Win64" }
            cppdialect( "c++20" ) -- Add something like Maximum Cpp version/dialect?
            defines( { "QW_TARGET_WIN64", "QW_CPP20" } )
            buildoptions { "/Zc:preprocessor" }

        -- May remove QW_CPP20 later.
    end,

    Module_Project = Default_Module_Setup( "Windows" )
}