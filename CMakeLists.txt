# Duck Object Library

# How to configure your CMakeLists.txt to add libduck as a dependency in another project

# Set the path to the location of the Duck project files (i.e. where this file lives)
# set( DUCK_PATH Absolute-Path-To-Duck-Project )
# get_filename_component( DUCK_PATH Relative-Path-To-Duck-Project ABSOLUTE )

# Set the destination path for public headers -- the headers will be copied into a 'Duck'
# subdirectory, making it easy to include Duck via #include <Duck/Duck.h>
# set( DUCK_PUBLIC_HEADERS_PATH Destination-Path-For-Public-Headers )

# Include this file in the build or add Duck as a subdirectory to import the 'DuckStaticLibrary'
# and 'DuckSharedLibrary' targets
# include( ${DUCK_PATH}/CMakeLists.txt )
# add_subdirectory( ${DUCK_PATH} ${PROJECT_BINARY_DIR} )

# Link against 'DuckStaticLibrary' or 'DuckSharedLibrary'
# target_include_directories( Your-Target PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
# target_link_libraries( Your-Target DuckSharedLibrary )


# Project Settings =======================================================================
cmake_minimum_required( VERSION 3.0 )

project( Duck )


# Configuration ==========================================================================

# Paths ----------------------------------------------------------------------------------
if( NOT DEFINED DUCK_PATH )
    set( DUCK_PATH ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

if( NOT DEFINED DUCK_PUBLIC_HEADERS_PATH )
    set( DUCK_PUBLIC_HEADERS_PATH ${CMAKE_CURRENT_BINARY_DIR}/Include )
endif()

if( NOT DEFINED DUCK_BUILD_STATIC
    AND NOT DEFINED DUCK_BUILD_SHARED
    AND NOT DEFINED DUCK_BUILD_FRAMEWORK )
    
    set( DUCK_BUILD_STATIC 1 )
    set( DUCK_BUILD_SHARED 1 )
    set( DUCK_BUILD_FRAMEWORK 1 )
endif()

# Library Name ---------------------------------------------------------------------------
if( WIN32 )
    if( DEBUG )
        set( DuckStaticLibraryName DuckStaticd )
        set( DuckSharedLibraryName Duckd )
    else()
        set( DuckStaticLibraryName DuckStatic )
        set( DuckSharedLibraryName Duck )
    endif()
else()
    set( DuckStaticLibraryName duck )
    set( DuckSharedLibraryName duck )
endif()

# External Dependencies ------------------------------------------------------------------
if( APPLE )

elseif( ANDROID )

elseif( UNIX )
    set( DuckRequiredLibraries  uuid )
    if( TARGET DuckStaticLibrary )
        target_link_libraries( DuckStaticLibrary uuid )
    endif()
    
    if( TARGET DuckSharedLibrary )
        target_link_libraries( DuckSharedLibrary uuid )
    endif()
    
elseif( WIN32 )
    
endif()

# Functions ==============================================================================

# Extract a list of headers from a list of files
function( get_header_files VarName Files )
    set( HeaderFiles Files )
    list( FILTER HeaderFiles INCLUDE REGEX "^.*\\.(h)$" )
    set( ${VarName} HeaderFiles PARENT_SCOPE )
endfunction()


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



# Source Files ===========================================================================
set( DuckSources
        ${DUCK_PATH}/Duck.h
        ${DUCK_PATH}/Source/DKAllocation.c
        ${DUCK_PATH}/Source/DKAllocation.h
        ${DUCK_PATH}/Source/DKArray.c
        ${DUCK_PATH}/Source/DKArray.h
        ${DUCK_PATH}/Source/DKBinaryTree.c
        ${DUCK_PATH}/Source/DKBinaryTree.h
        ${DUCK_PATH}/Source/DKBitList.c
        ${DUCK_PATH}/Source/DKBitList.h
        ${DUCK_PATH}/Source/DKBoolean.c
        ${DUCK_PATH}/Source/DKBoolean.h
        ${DUCK_PATH}/Source/DKBuffer.c
        ${DUCK_PATH}/Source/DKBuffer.h
        ${DUCK_PATH}/Source/DKByteArray.c
        ${DUCK_PATH}/Source/DKByteArray.h
        ${DUCK_PATH}/Source/DKCollection.c
        ${DUCK_PATH}/Source/DKCollection.h
        ${DUCK_PATH}/Source/DKComparison.c
        ${DUCK_PATH}/Source/DKComparison.h
        ${DUCK_PATH}/Source/DKCondition.c
        ${DUCK_PATH}/Source/DKCondition.h
        ${DUCK_PATH}/Source/DKConfig.h
        ${DUCK_PATH}/Source/DKConversion.c
        ${DUCK_PATH}/Source/DKConversion.h
        ${DUCK_PATH}/Source/DKCopying.c
        ${DUCK_PATH}/Source/DKCopying.h
        ${DUCK_PATH}/Source/DKData.c
        ${DUCK_PATH}/Source/DKData.h
        ${DUCK_PATH}/Source/DKDescription.c
        ${DUCK_PATH}/Source/DKDescription.h
        ${DUCK_PATH}/Source/DKDictionary.c
        ${DUCK_PATH}/Source/DKDictionary.h
        ${DUCK_PATH}/Source/DKEgg.c
        ${DUCK_PATH}/Source/DKEgg.h
        ${DUCK_PATH}/Source/DKEncoding.c
        ${DUCK_PATH}/Source/DKEncoding.h
        ${DUCK_PATH}/Source/DKEnum.c
        ${DUCK_PATH}/Source/DKEnum.h
        ${DUCK_PATH}/Source/DKFile.c
        ${DUCK_PATH}/Source/DKFile.h
        ${DUCK_PATH}/Source/DKGenericArray.c
        ${DUCK_PATH}/Source/DKGenericArray.h
        ${DUCK_PATH}/Source/DKGenericHashTable.c
        ${DUCK_PATH}/Source/DKGenericHashTable.h
        ${DUCK_PATH}/Source/DKGraph.c
        ${DUCK_PATH}/Source/DKGraph.h
        ${DUCK_PATH}/Source/DKHashTable.c
        ${DUCK_PATH}/Source/DKHashTable.h
        ${DUCK_PATH}/Source/DKJSON.c
        ${DUCK_PATH}/Source/DKJSON.h
        ${DUCK_PATH}/Source/DKLinkedList.c
        ${DUCK_PATH}/Source/DKLinkedList.h
        ${DUCK_PATH}/Source/DKList.c
        ${DUCK_PATH}/Source/DKList.h
        ${DUCK_PATH}/Source/DKLocking.c
        ${DUCK_PATH}/Source/DKLocking.h
        ${DUCK_PATH}/Source/DKMutex.c
        ${DUCK_PATH}/Source/DKMutex.h
        ${DUCK_PATH}/Source/DKNodePool.c
        ${DUCK_PATH}/Source/DKNodePool.h
        ${DUCK_PATH}/Source/DKNumber.c
        ${DUCK_PATH}/Source/DKNumber.h
        ${DUCK_PATH}/Source/DKPair.c
        ${DUCK_PATH}/Source/DKPair.h
        ${DUCK_PATH}/Source/DKPlatform.c
        ${DUCK_PATH}/Source/DKPlatform.h
        ${DUCK_PATH}/Source/DKPredicate.c
        ${DUCK_PATH}/Source/DKPredicate.h
        ${DUCK_PATH}/Source/DKReadWriteLock.c
        ${DUCK_PATH}/Source/DKReadWriteLock.h
        ${DUCK_PATH}/Source/DKRuntime.c
        ${DUCK_PATH}/Source/DKRuntime.h
        ${DUCK_PATH}/Source/DKRuntime+Interfaces.c
        ${DUCK_PATH}/Source/DKRuntime+Interfaces.h
        ${DUCK_PATH}/Source/DKRuntime+Metadata.c
        ${DUCK_PATH}/Source/DKRuntime+Metadata.h
        ${DUCK_PATH}/Source/DKRuntime+Properties.c
        ${DUCK_PATH}/Source/DKRuntime+Properties.h
        ${DUCK_PATH}/Source/DKRuntime+RefCount.c
        ${DUCK_PATH}/Source/DKRuntime+RefCount.h
        ${DUCK_PATH}/Source/DKRuntime+Reflection.c
        ${DUCK_PATH}/Source/DKRuntime+Reflection.h
        ${DUCK_PATH}/Source/DKRuntime+Symbols.c
        ${DUCK_PATH}/Source/DKSemaphore.c
        ${DUCK_PATH}/Source/DKSemaphore.h
        ${DUCK_PATH}/Source/DKSet.c
        ${DUCK_PATH}/Source/DKSet.h
        ${DUCK_PATH}/Source/DKShell.c
        ${DUCK_PATH}/Source/DKShell.h
        ${DUCK_PATH}/Source/DKStream.c
        ${DUCK_PATH}/Source/DKStream.h
        ${DUCK_PATH}/Source/DKString.c
        ${DUCK_PATH}/Source/DKString.h
        ${DUCK_PATH}/Source/DKStruct.c
        ${DUCK_PATH}/Source/DKStruct.h
        ${DUCK_PATH}/Source/DKThread.c
        ${DUCK_PATH}/Source/DKThread.h
        ${DUCK_PATH}/Source/DKThreadPool.c
        ${DUCK_PATH}/Source/DKThreadPool.h
        ${DUCK_PATH}/Source/DKUnicode.c
        ${DUCK_PATH}/Source/DKUnicode.h
        ${DUCK_PATH}/Source/DKXML.c
        ${DUCK_PATH}/Source/DKXML.h )

set( DuckICU
        ${DUCK_PATH}/ThirdParty/icu/uassert.h
        ${DUCK_PATH}/ThirdParty/icu/utf_impl.c
        ${DUCK_PATH}/ThirdParty/icu/unicode/platform.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/ptypes.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/uconfig.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/umachine.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/urename.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utf_old.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utf.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utf8.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utf16.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utf32.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/utypes.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/uvernum.h
        ${DUCK_PATH}/ThirdParty/icu/unicode/uversion.h )

set( DuckYXML
        ${DUCK_PATH}/ThirdParty/yxml/yxml.c
        ${DUCK_PATH}/ThirdParty/yxml/yxml.h )

set( DuckPublicHeaderSearchPaths
        ${DUCK_PATH}
        ${DUCK_PATH}/Source )

set( DuckProjectHeaderSearchPaths
        ${DUCK_PATH}/ThirdParty
        ${DUCK_PATH}/ThirdParty/icu )

source_group( TREE ${DUCK_PATH} FILES ${DuckSources} ${DuckICU} ${DuckYXML} )


# Copy Public Header Files ===============================================================
if( DUCK_BUILD_STATIC OR DUCK_BUILD_SHARED )
    add_copy_target( DuckCopyPublicHeaders ${DUCK_PUBLIC_HEADERS_PATH}/Duck )
    add_copy_headers( DuckCopyPublicHeaders ${DuckSources} )
endif()


# Build Libraries ========================================================================
function( add_duck_library TargetName TargetType FRAMEWORK LibraryName )
    add_library( ${TargetName} ${TargetType}
        ${DuckSources}
        ${DuckICU}
        ${DuckYXML} )

    set_target_properties( ${TargetName} PROPERTIES
        OUTPUT_NAME ${LibraryName} 
        LINKER_LANGUAGE C )

    if( FRAMEWORK )
        set_target_properties( DuckFramework PROPERTIES
            FRAMEWORK 1 
            MACOSX_FRAMEWORK_INFO_PLIST ${DUCK_PATH}/Info.plist )
    endif()

    add_target_headers( ${TargetName}
        PUBLIC "${DuckSources}"
        PROJECT "${DuckICU}" "${DuckYXML}" )

    target_include_directories( ${TargetName}
        PUBLIC ${DuckPublicHeaderSearchPaths}
        PRIVATE ${DuckProjectHeaderSearchPaths} )

    target_link_libraries( ${TargetName} "${DuckRequiredLibraries}" )

    if( NOT FRAMEWORK )
        add_dependencies( ${TargetName} DuckCopyPublicHeaders )
    endif()

    if( NOT ANDROID AND NOT FRAMEWORK )
        install( TARGETS ${TargetName}
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                PUBLIC_HEADER DESTINATION include/Duck )
    endif()
endfunction()

# Static Library -------------------------------------------------------------------------
if( DUCK_BUILD_STATIC )
    add_duck_library( DuckStaticLibrary STATIC NO ${DuckStaticLibraryName}  )
endif()

# Shared Library -------------------------------------------------------------------------
if( DUCK_BUILD_SHARED )
    add_duck_library( DuckSharedLibrary SHARED NO ${DuckSharedLibraryName}  )
endif()

# Framework ------------------------------------------------------------------------------
if( APPLE AND DUCK_BUILD_FRAMEWORK )
    add_duck_library( DuckFramework SHARED YES "Duck" )
endif()


