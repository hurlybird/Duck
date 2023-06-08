# BuildTools.cmake =======================================================================

# Various utility functions for CMake builds

include_guard( DIRECTORY )


# General Stuff ==========================================================================

# Split a list of files into headers and sources
function( append_headers_and_sources HeadersVar SourcesVar )
    set( HeaderFiles "" )
    set( SourceFiles "" )
    
    foreach( argi ${ARGN} )
        if( ${argi} MATCHES "^.*\\.(h)$" )
            list( APPEND HeaderFiles ${argi} )
        else()
            list( APPEND SourceFiles ${argi} )
        endif()
    endforeach()
    
    set( ${HeadersVar} ${${HeadersVar}} ${HeaderFiles} PARENT_SCOPE )
    set( ${SourcesVar} ${${SourcesVar}} ${SourceFiles} PARENT_SCOPE )
endfunction()


# Copy Targets ===========================================================================

# Copy files to a destination
function( copy_files TargetName DestinationDir )
    set( DstFiles "" )
    set( DstPaths "" )
    set( DstPath ${DestinationDir} )

    set( i 0 )
    list( LENGTH ARGN count )

    while( ${i} LESS ${count} )
        list( GET ARGN ${i} srci )

        if( ${srci} STREQUAL "SUBDIRECTORY" )
            math( EXPR i ${i}+1 )
            list( GET ARGN ${i} srci )
            set( DstPath ${DestinationDir}/${srci} )
            set( DstPaths ${DstPaths} ${DstPath} )
        else()
            get_filename_component( filename ${srci} NAME )
            set( dsti ${DstPath}/${filename} )
            set( DstFiles ${DstFiles} ${dsti} )
            add_custom_command(
                OUTPUT ${dsti}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${srci} ${DstPath}
                DEPENDS ${srci}
                COMMENT "Copying ${filename}" )
        endif()
        math( EXPR i ${i}+1 )
    endwhile()

    add_custom_target( ${TargetName}
        #COMMENT "Copying files to ${DestinationDir}..."
        DEPENDS ${DstFiles} )

    # Create destination(s) (works for Visual Studio, Visual Studio Code, Xcode)
    if( ${CMAKE_GENERATOR} MATCHES "^Visual Studio*" OR ${CMAKE_GENERATOR} MATCHES "^Xcode*" )
        add_custom_command( TARGET ${TargetName}
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${DestinationDir} )

        foreach( path ${DstPaths} )
            add_custom_command( TARGET ${TargetName}
                PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${path} )
        endforeach()

    # Create destination(s) when running cmake (for CLion, make)
    else()
        file( MAKE_DIRECTORY ${DestinationDir} )

        foreach( path ${DstPaths} )
            file( MAKE_DIRECTORY ${path} )
        endforeach()
    endif()
endfunction()


# Copy header files to a destination
function( copy_header_files TargetName DestinationDir )
    set( DstFiles "" )

    foreach( srci ${ARGN} )
        if( ${srci} MATCHES "^.*\\.(h)$" )
            get_filename_component( filename ${srci} NAME )
            set( dsti ${DestinationDir}/${filename} )
            set( DstFiles ${DstFiles} ${dsti} )
            add_custom_command(
                OUTPUT ${dsti}
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${srci} ${DestinationDir}
                DEPENDS ${srci}
                COMMENT "Copying ${filename}" )
        endif()
    endforeach()

    add_custom_target( ${TargetName}
        #COMMENT "Copying header files to ${DestinationDir}..."
        DEPENDS ${DstFiles} )

    # Create destination (works for Visual Studio, Visual Studio Code, Xcode)
    if( ${CMAKE_GENERATOR} MATCHES "^Visual Studio*" OR ${CMAKE_GENERATOR} MATCHES "^Xcode*" )
        add_custom_command( TARGET ${TargetName}
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${DestinationDir} )

    # Create destination when running cmake (for CLion, make)
    else()
        file( MAKE_DIRECTORY ${DestinationDir} )
    endif()
endfunction()


# Target Headers/Sources =================================================================

# Add headers to a target (also adds the headers as sources)
function( add_target_headers TargetName )
    set( scope "PUBLIC" )
    foreach( argi ${ARGN} )
        if( ${argi} STREQUAL "PUBLIC" )
            set( scope "PUBLIC" )
        elseif( ${argi} STREQUAL "PRIVATE" )
            set( scope "PUBLIC" )
        elseif( ${argi} STREQUAL "PROJECT" )
            set( scope "PROJECT" )
        elseif( ${argi} MATCHES "^.*\\.(h)$" )
            if( ${scope} STREQUAL "PUBLIC" )
                set_property( TARGET ${TargetName} APPEND PROPERTY PUBLIC_HEADER "${argi}" )
            elseif( ${scope} STREQUAL "PRIVATE" )
                set_property( TARGET ${TargetName} APPEND PROPERTY PRIVATE_HEADER "${argi}" )
            endif()
            set_property( TARGET ${TargetName} APPEND PROPERTY SOURCES "${argi}" )
        endif()
    endforeach()
endfunction()


# Target Properties ======================================================================

# Add a property to a target if it isn't already set
function( add_target_property TargetName PropertyName PropertyValue )
    get_target_property( CurrentValue ${TargetName} ${PropertyName} )

    if( (CurrentValue STREQUAL "") OR (CurrentValue STREQUAL "${PropertyName}-NOTFOUND") )
        set_property( TARGET ${TargetName} APPEND PROPERTY ${PropertyName} ${PropertyValue} )
    endif()
endfunction()



