# Duck Object Library

# How to configure your CMakeLists.txt to add libduck as a dependency in another project

# Set the path to the location of the Duck project files (i.e. where this file lives)
# set( DUCK_PATH Absolute-Path-To-Duck-Project )
# get_filename_component( DUCK_PATH Relative-Path-To-Duck-Project ABSOLUTE )

# Set the destination path for public headers -- the headers will be copied into a 'Duck'
# subdirectory, making it easy to include Duck via #include <Duck/Duck.h>
# set( DUCK_PUBLIC_HEADERS_PATH Destination-Path-For-Public-Headers )

# Include this file in the build or add Duck as a subdirectory to import the 'duck-static'
# and 'duck-shared' targets
# include( ${DUCK_PATH}/CMakeLists.txt )
# add_subdirectory( ${DUCK_PATH} ${PROJECT_BINARY_DIR} )

# Link against 'duck-static' or 'duck-shared'
# target_include_directories( Your-Target PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
# target_link_libraries( Your-Target duck-shared )


# Project Settings =======================================================================
cmake_minimum_required( VERSION 3.0 )

project( Duck )


# Setup Paths ============================================================================
if( NOT DEFINED DUCK_PATH )
    set( DUCK_PATH ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

if( NOT DEFINED DUCK_PUBLIC_HEADERS_PATH )
    set( DUCK_PUBLIC_HEADERS_PATH ${CMAKE_CURRENT_BINARY_DIR}/Include )
endif()

file( MAKE_DIRECTORY ${DUCK_PUBLIC_HEADERS_PATH} )


# Functions ==============================================================================
function( AddCopyFilesTarget TargetName SourceDirs Patterns DestinationDir )
    add_custom_target( ${TargetName} )

    file( MAKE_DIRECTORY ${DestinationDir} )

    foreach( SourceDir ${SourceDirs} )
        foreach( Pattern ${Patterns} )
            file( GLOB Files ${SourceDir}/${Pattern} )

            foreach( File ${Files} )
                add_custom_command( TARGET ${TargetName} PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E
                        copy_if_different  ${File} ${DestinationDir} )
            endforeach()
        endforeach()
    endforeach()
endfunction()


function( AddPublicHeaderFiles TargetName HeaderDirs )
    foreach( HeaderDir ${HeaderDirs} )
        file( GLOB Headers ${HeaderDir}/*.h )
        set_property( TARGET ${TargetName} APPEND PROPERTY PUBLIC_HEADER "${Headers}" )
    endforeach()
endfunction()



# Source Files ===========================================================================
set( DuckSources
        # ${DUCK_PATH}/Source/DKAllocation.c
        ${DUCK_PATH}/Source/DKArray.c
        ${DUCK_PATH}/Source/DKBinaryTree.c
        ${DUCK_PATH}/Source/DKBitList.c
        ${DUCK_PATH}/Source/DKBoolean.c
        ${DUCK_PATH}/Source/DKBuffer.c
        ${DUCK_PATH}/Source/DKByteArray.c
        ${DUCK_PATH}/Source/DKCollection.c
        ${DUCK_PATH}/Source/DKComparison.c
        ${DUCK_PATH}/Source/DKCondition.c
        ${DUCK_PATH}/Source/DKConversion.c
        ${DUCK_PATH}/Source/DKCopying.c
        ${DUCK_PATH}/Source/DKData.c
        ${DUCK_PATH}/Source/DKDescription.c
        ${DUCK_PATH}/Source/DKDictionary.c
        ${DUCK_PATH}/Source/DKEgg.c
        ${DUCK_PATH}/Source/DKEncoding.c
        ${DUCK_PATH}/Source/DKEnum.c
        ${DUCK_PATH}/Source/DKFile.c
        ${DUCK_PATH}/Source/DKGenericArray.c
        ${DUCK_PATH}/Source/DKGenericHashTable.c
        ${DUCK_PATH}/Source/DKGraph.c
        ${DUCK_PATH}/Source/DKHashTable.c
        ${DUCK_PATH}/Source/DKJSON.c
        ${DUCK_PATH}/Source/DKLinkedList.c
        ${DUCK_PATH}/Source/DKList.c
        ${DUCK_PATH}/Source/DKLocking.c
        ${DUCK_PATH}/Source/DKMutex.c
        ${DUCK_PATH}/Source/DKNodePool.c
        ${DUCK_PATH}/Source/DKNumber.c
        ${DUCK_PATH}/Source/DKPair.c
        ${DUCK_PATH}/Source/DKPlatform.c
        ${DUCK_PATH}/Source/DKPredicate.c
        ${DUCK_PATH}/Source/DKReadWriteLock.c
        ${DUCK_PATH}/Source/DKRuntime.c
        ${DUCK_PATH}/Source/DKRuntime+Interfaces.c
        ${DUCK_PATH}/Source/DKRuntime+Metadata.c
        ${DUCK_PATH}/Source/DKRuntime+Properties.c
        ${DUCK_PATH}/Source/DKRuntime+RefCount.c
        ${DUCK_PATH}/Source/DKRuntime+Reflection.c
        ${DUCK_PATH}/Source/DKRuntime+Symbols.c
        ${DUCK_PATH}/Source/DKSemaphore.c
        ${DUCK_PATH}/Source/DKSet.c
        ${DUCK_PATH}/Source/DKShell.c
        ${DUCK_PATH}/Source/DKStream.c
        ${DUCK_PATH}/Source/DKString.c
        ${DUCK_PATH}/Source/DKStruct.c
        ${DUCK_PATH}/Source/DKThread.c
        ${DUCK_PATH}/Source/DKThreadPool.c
        ${DUCK_PATH}/Source/DKUnicode.c
        ${DUCK_PATH}/Source/DKXML.c )

set( DuckICU
        ${DUCK_PATH}/ThirdParty/icu/utf_impl.c )

set( DuckYXML
        ${DUCK_PATH}/ThirdParty/yxml/yxml.c )

set( DuckPublicHeaderSearchPaths
        ${DUCK_PATH}
        ${DUCK_PATH}/Source )

set( DuckPrivateHeaderSearchPaths
        ${DUCK_PATH}/ThirdParty
        ${DUCK_PATH}/ThirdParty/icu )


# Copy Public Header Files ===============================================================
AddCopyFilesTarget( DuckCopyPublicHeaders "${DuckPublicHeaderSearchPaths}" "*.h" ${DUCK_PUBLIC_HEADERS_PATH}/Duck )


# Static and Shared Library Targets ======================================================
add_library( duck-static STATIC
        ${DuckSources}
        ${DuckICU}
        ${DuckYXML} )

add_library( duck-shared SHARED
        ${DuckSources}
        ${DuckICU}
        ${DuckYXML} )

add_dependencies( duck-static DuckCopyPublicHeaders )
add_dependencies( duck-shared DuckCopyPublicHeaders )


# Target Properties ======================================================================
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

set_target_properties( duck-static PROPERTIES OUTPUT_NAME ${DuckStaticLibraryName} )
set_target_properties( duck-static PROPERTIES LINKER_LANGUAGE C )

set_target_properties( duck-shared PROPERTIES OUTPUT_NAME ${DuckSharedLibraryName} )
set_target_properties( duck-shared PROPERTIES LINKER_LANGUAGE C )

AddPublicHeaderFiles( duck-static "${DuckPublicHeaderSearchPaths}" )
AddPublicHeaderFiles( duck-shared "${DuckPublicHeaderSearchPaths}" )


# Header Search Paths ====================================================================
target_include_directories( duck-static
        PUBLIC ${DuckPublicHeaderSearchPaths}
        PRIVATE ${DuckPrivateHeaderSearchPaths} )

target_include_directories( duck-shared
        PUBLIC ${DuckPublicHeaderSearchPaths}
        PRIVATE ${DuckPrivateHeaderSearchPaths} )


# Link Libraries =========================================================================
if( APPLE )

elseif( ANDROID )

elseif( UNIX )
    target_link_libraries( duck-static uuid )
    target_link_libraries( duck-shared uuid )
    
elseif( WIN32 )
    
endif()


# Install ================================================================================
if( NOT ANDROID )
    install( TARGETS duck-static duck-shared
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include/Duck )
endif()
