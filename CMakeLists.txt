# Duck Object Library

# How to configure your CMakeLists.txt to add libduck as a dependency in another project

# Set the path to the location of the Duck project files (i.e. where this file lives)
# set( DUCK_PATH Absolute-Path-To-Duck-Project )
# get_filename_component( DUCK_PATH Relative-Path-To-Duck-Project ABSOLUTE )

# Set the destination path for public headers -- the headers will be copied into a 'Duck'
# subdirectory, making it easy to include Duck via #include <Duck/Duck.h>
# set( DUCK_PUBLIC_HEADERS_PATH Destination-Path-For-Public-Headers )

# Include this file in the build to import the 'duck-static' and 'duck-shared' targets
# include( ${DUCK_PATH}/CMakeLists.txt )

# Link against 'duck-static' or 'duck-shared'
# target_include_directories( Your-Target PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
# target_link_libraries( Your-Target duck-shared )


# Project Settings =======================================================================
cmake_minimum_required( VERSION 3.0 )

project( Duck )


# Setup Paths ============================================================================
if( DEFINED DUCK_PATH )
    set( DuckProjectDir ${DUCK_PATH} )
else()
    set( DuckProjectDir ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

if( DEFINED DUCK_PUBLIC_HEADERS_PATH )
    set( DuckPublicHeadersDir ${DUCK_PUBLIC_HEADERS_PATH} )
else()
    set( DuckPublicHeadersDir ${PROJECT_BINARY_DIR}/Include )
endif()

file( MAKE_DIRECTORY ${DuckPublicHeadersDir} )


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
        # ${DuckProjectDir}/Source/DKAllocation.c
        ${DuckProjectDir}/Source/DKArray.c
        ${DuckProjectDir}/Source/DKBinaryTree.c
        ${DuckProjectDir}/Source/DKBitList.c
        ${DuckProjectDir}/Source/DKBoolean.c
        ${DuckProjectDir}/Source/DKBuffer.c
        ${DuckProjectDir}/Source/DKByteArray.c
        ${DuckProjectDir}/Source/DKCollection.c
        ${DuckProjectDir}/Source/DKComparison.c
        ${DuckProjectDir}/Source/DKCondition.c
        ${DuckProjectDir}/Source/DKConversion.c
        ${DuckProjectDir}/Source/DKCopying.c
        ${DuckProjectDir}/Source/DKData.c
        ${DuckProjectDir}/Source/DKDescription.c
        ${DuckProjectDir}/Source/DKDictionary.c
        ${DuckProjectDir}/Source/DKEgg.c
        ${DuckProjectDir}/Source/DKEncoding.c
        ${DuckProjectDir}/Source/DKEnum.c
        ${DuckProjectDir}/Source/DKFile.c
        ${DuckProjectDir}/Source/DKGenericArray.c
        ${DuckProjectDir}/Source/DKGenericHashTable.c
        ${DuckProjectDir}/Source/DKGraph.c
        ${DuckProjectDir}/Source/DKHashTable.c
        ${DuckProjectDir}/Source/DKJSON.c
        ${DuckProjectDir}/Source/DKLinkedList.c
        ${DuckProjectDir}/Source/DKList.c
        ${DuckProjectDir}/Source/DKLocking.c
        ${DuckProjectDir}/Source/DKMutex.c
        ${DuckProjectDir}/Source/DKNodePool.c
        ${DuckProjectDir}/Source/DKNumber.c
        ${DuckProjectDir}/Source/DKPair.c
        ${DuckProjectDir}/Source/DKPlatform.c
        ${DuckProjectDir}/Source/DKPredicate.c
        ${DuckProjectDir}/Source/DKReadWriteLock.c
        ${DuckProjectDir}/Source/DKRuntime.c
        ${DuckProjectDir}/Source/DKRuntime+Interfaces.c
        ${DuckProjectDir}/Source/DKRuntime+Metadata.c
        ${DuckProjectDir}/Source/DKRuntime+Properties.c
        ${DuckProjectDir}/Source/DKRuntime+RefCount.c
        ${DuckProjectDir}/Source/DKRuntime+Reflection.c
        ${DuckProjectDir}/Source/DKRuntime+Symbols.c
        ${DuckProjectDir}/Source/DKSemaphore.c
        ${DuckProjectDir}/Source/DKSet.c
        ${DuckProjectDir}/Source/DKShell.c
        ${DuckProjectDir}/Source/DKStream.c
        ${DuckProjectDir}/Source/DKString.c
        ${DuckProjectDir}/Source/DKStruct.c
        ${DuckProjectDir}/Source/DKThread.c
        ${DuckProjectDir}/Source/DKThreadPool.c
        ${DuckProjectDir}/Source/DKUnicode.c
        ${DuckProjectDir}/Source/DKXML.c )

set( DuckICU
        ${DuckProjectDir}/ThirdParty/icu/utf_impl.c )

set( DuckYXML
        ${DuckProjectDir}/ThirdParty/yxml/yxml.c )

set( DuckPublicHeaderSearchPaths
        ${DuckProjectDir}
        ${DuckProjectDir}/Source )

set( DuckPrivateHeaderSearchPaths
        ${DuckProjectDir}/ThirdParty
        ${DuckProjectDir}/ThirdParty/icu )


# Copy Public Header Files ===============================================================
AddCopyFilesTarget( DuckCopyPublicHeaders "${DuckPublicHeaderSearchPaths}" "*.h" ${DuckPublicHeadersDir}/Duck )


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
if( UNIX )
    target_link_libraries( duck-static uuid )
    target_link_libraries( duck-shared uuid )
endif()


# Install ================================================================================
if( NOT ANDROID )
    install( TARGETS duck-static duck-shared
            ARCHIVE DESTINATION lib
            LIBRARY DESTINATION lib
            PUBLIC_HEADER DESTINATION include/Duck )
endif()
