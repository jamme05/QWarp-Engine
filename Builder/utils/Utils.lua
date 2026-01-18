function CmakePreBuild( dependency, cmake_path, settings )
    local dependency_path = "./Build/" .. dependency .. "/cmake"
    local return_path   = os.realpath( "./" )

    if( cmake_path == nil ) then
        cmake_path = ""
    end

    os.mkdir( dependency_path )
    os.chdir( dependency_path )
    print( "Building " .. dependency .. " files..." )
    os.execute( "cmake " .. path.join( return_path, dependency, cmake_path ) )
    if( build_config ~= nil ) then
        os.execute( "cmake " .. settings .. "-B build" )
    end
    print( "Build done.\n" )
    os.chdir( return_path )

end
function CMakeBuilder( dependency, cmake_path, build_config )
    local dependency_path = "./Build/" .. dependency .. "/cmake"
    local return_path   = os.realpath( "./" )

    if( cmake_path == nil ) then
        cmake_path = ""
    end

    os.mkdir( dependency_path )
    os.chdir( dependency_path )
    print( "Building " .. dependency .. " files..." )
    os.execute( "cmake " .. path.join( return_path, dependency, cmake_path ) )
    if( build_config ~= nil ) then
        os.execute( "cmake --build . --config " .. build_config )
    end
    print( "Build done.\n" )
    os.chdir( return_path )
end

function CopyFolder( source, destination )
    local pattern = path.join( source, "**" )

    local dirs  = os.matchdirs( pattern )
    local files = os.matchfiles( pattern )

    for _, dir in pairs( dirs ) do
        os.mkdir( path.join( destination, path.getrelative( source, dir ) ) )
    end

    for _, file in pairs( files ) do
        local ok, err = os.copyfile( file, path.join( destination, path.getrelative( source, file ) ) )
        if( not ok ) then
            print( err )
        end
    end
end