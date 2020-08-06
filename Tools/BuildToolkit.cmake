# BuildTools.cmake =======================================================================

# Various utility functions for CMake builds

include_guard( DIRECTORY )


# General Stuff ==========================================================================

# Extract a list of headers from a list of files
function( get_header_files VarName Files )
    set( HeaderFiles Files )
    list( FILTER HeaderFiles INCLUDE REGEX "^.*\\.(h)$" )
    set( ${VarName} HeaderFiles PARENT_SCOPE )
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
function( add_copy_files TargetName )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        add_custom_command( TARGET ${TargetName} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${argi} ${DestinationDir} )
    endforeach()
endfunction()


# Add header files to a copy target
function( add_copy_headers TargetName )
    get_target_property( DestinationDir ${TargetName} "COPY_DESTINATION" )
    foreach( argi ${ARGN} )
        if( ${argi} MATCHES "^.*\\.(h)$" )
            add_custom_command( TARGET ${TargetName} PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${argi} ${DestinationDir} )
        endif()
    endforeach()
endfunction()


# Add files matching a glob pattern to a copy target
function( add_copy_directories TargetName Patterns )
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

# function( add_copy_files_target TargetName Sources Patterns DestinationDir )
#     add_custom_target( ${TargetName} )
# 
#     add_custom_command( TARGET ${TargetName} PRE_BUILD
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${DestinationDir} )
# 
#     foreach( Source ${Sources} )
#         if( IS_DIRECTORY Source )
#             foreach( Pattern ${Patterns} )
#                 file( GLOB Files ${Source}/${Pattern} )
# 
#                 foreach( File ${Files} )
#                     add_custom_command( TARGET ${TargetName} PRE_BUILD
#                         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${File} ${DestinationDir} )
#                 endforeach()
#             endforeach()
#         elseif( EXISTS Source )
#             add_custom_command( TARGET ${TargetName} PRE_BUILD
#                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Source} ${DestinationDir} )
#         endif()
#     endforeach()
# endfunction()


# Target Headers =========================================================================

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


# function( set_public_headers TargetName HeaderDirs )
#     foreach( HeaderDir ${HeaderDirs} )
#         file( GLOB Headers ${HeaderDir}/*.h )
#         set_property( TARGET ${TargetName} APPEND PROPERTY PUBLIC_HEADER "${Headers}" )
#         set_property( TARGET ${TargetName} APPEND PROPERTY SOURCES "${Headers}" )
#     endforeach()
# endfunction()
# 
# 
# function( set_private_headers TargetName HeaderDirs )
#     foreach( HeaderDir ${HeaderDirs} )
#         file( GLOB Headers ${HeaderDir}/*.h )
#         set_property( TARGET ${TargetName} APPEND PROPERTY PRIVATE_HEADER "${Headers}" )
#         set_property( TARGET ${TargetName} APPEND PROPERTY SOURCES "${Headers}" )
#     endforeach()
# endfunction()
# 
# 
# function( set_project_headers TargetName HeaderDirs )
#     foreach( HeaderDir ${HeaderDirs} )
#         file( GLOB Headers ${HeaderDir}/*.h )
#         set_property( TARGET ${TargetName} APPEND PROPERTY SOURCES "${Headers}" )
#     endforeach()
# endfunction()
