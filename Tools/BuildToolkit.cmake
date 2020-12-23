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

# Create a copy target
function( add_copy_target TargetName DestinationDir )
    add_custom_target( ${TargetName} )
    set_target_properties( ${TargetName} PROPERTIES "COPY_DESTINATION" ${DestinationDir} )

    add_custom_command( TARGET ${TargetName} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DestinationDir} )
endfunction()


# Add files to a copy target
function( target_copy_files TargetName )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        add_custom_command( TARGET ${TargetName} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${argi} ${DestinationDir} )
    endforeach()
endfunction()


# Add header files to a copy target
function( target_copy_headers TargetName )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        if( ${argi} MATCHES "^.*\\.(h)$" )
            add_custom_command( TARGET ${TargetName} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${argi} ${DestinationDir} )
        endif()
    endforeach()
endfunction()


# Add directories to a copy target
function( target_copy_directories TargetName )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        add_custom_command( TARGET ${TargetName} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${argi} ${DestinationDir} )
    endforeach()
endfunction()


# Add files matching a glob pattern to a copy target
function( target_copy_matching_in_directories TargetName Patterns )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        foreach( Pattern ${Patterns} )
            file( GLOB Files ${argi}/${Pattern} )

            foreach( File ${Files} )
                add_custom_command( TARGET ${TargetName} PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${File} ${DestinationDir} )
            endforeach()
        endforeach()
    endforeach()
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



