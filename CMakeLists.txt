# Duck Object Library

# How to add libduck to your CMakeLists.txt build.

# (Optional) - Set the path to the Duck project files (i.e. where this file lives). This
# is mainly useful if you're adding libduck via include vs. add_subdirectory.
#
# set( DUCK_PATH Absolute-Path-To-Duck-Project )
# get_filename_component( DUCK_PATH Relative-Path-To-Duck-Project ABSOLUTE )

# (Optional) - Set the build variants. Valid values are:
#   SHARED -    build the shared library and public headers
#   STATIC -    build the static library and public headers
#   FRAMEWORK - build the framework (on Apple platforms)
#
# If not specified, the default is to build everything.
#
# set( DUCK_BUILD STATIC SHARED )

# Set the destination path for public headers. The public headers will be copied into a
# 'Duck' subdirectory of the destination, making it easy to include the Duck umbrella
# header via #include <Duck/Duck.h>. The default is to copy the headers to 'Include/Duck'
# in the current binary output directory.
#
# set( DUCK_PUBLIC_HEADERS_PATH Destination-Path-For-Public-Headers )

# Include this file in the build or add Duck as a subdirectory.
#
# include( ${DUCK_PATH}/CMakeLists.txt )
# add_subdirectory( ${DUCK_PATH} ${PROJECT_BINARY_DIR}/Duck )

# Link against 'DuckStaticLibrary', 'DuckSharedLibrary' or 'DuckFramework'.
#
# target_include_directories( Your-Target PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
# target_link_libraries( Your-Target DuckSharedLibrary )


# Project Settings =======================================================================
cmake_minimum_required( VERSION 3.16 )

set( CMAKE_SUPPRESS_REGENERATION ON )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

project( Duck )


# Configuration ==========================================================================
set( CMAKE_POSITION_INDEPENDENT_CODE 1 )

# Paths ----------------------------------------------------------------------------------
set( DUCK_PATH ${CMAKE_CURRENT_SOURCE_DIR} )

if( NOT DEFINED DUCK_PUBLIC_HEADERS_PATH )
    set( DUCK_PUBLIC_HEADERS_PATH ${CMAKE_CURRENT_BINARY_DIR}/Include )
endif()

if( NOT DEFINED DUCK_BINARY_PATH )
    set( DUCK_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR} )
endif()

# Fix Windows paths
string( REPLACE "\\" "/" DUCK_PATH ${DUCK_PATH} )
string( REPLACE "\\" "/" DUCK_PUBLIC_HEADERS_PATH ${DUCK_PUBLIC_HEADERS_PATH} )


# Build Variants -------------------------------------------------------------------------
if( NOT DEFINED DUCK_BUILD )
    set( DUCK_BUILD STATIC SHARED FRAMEWORK )
endif()


# Includes -------------------------------------------------------------------------------
include( ${DUCK_PATH}/Tools/BuildToolkit.cmake )


# Library Name ---------------------------------------------------------------------------
if( NX )
    set( DuckStaticLibraryName duck )
    set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )
    set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )
elseif( WIN32 )
    set( DuckStaticLibraryName DuckStatic )
    set( DuckSharedLibraryName Duck )
else()
    set( DuckStaticLibraryName duck )
    set( DuckSharedLibraryName duck )
    set( DuckFrameworkName Duck )
endif()


# External Dependencies ------------------------------------------------------------------
if( APPLE )

elseif( ANDROID )

elseif( NX )

elseif( UNIX )
    set( DuckRequiredLibraries
        pthread
        uuid )

elseif( WIN32 )
    set( DuckRequiredLibraries
        rpcrt4 )
    
endif()


# Source Files ===========================================================================
set( DuckSources
    ${DUCK_PATH}/Duck.h
    #${DUCK_PATH}/Source/DKAllocation.c -- empty file
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


# Add Support for Proprietary Platforms ==================================================
if( NX )
    include( ${DUCK_PATH}/Proprietary/NX/CMakeLists.txt )
endif()


# Configure Source Groups ================================================================
source_group( TREE ${DUCK_PATH} FILES ${DuckSources} ${DuckICU} ${DuckYXML} )


# Copy Public Header Files ===============================================================
if( STATIC IN_LIST DUCK_BUILD OR SHARED IN_LIST DUCK_BUILD )
    copy_header_files( DuckCopyPublicHeaders ${DUCK_PUBLIC_HEADERS_PATH}/Duck ${DuckSources} )
    set_target_properties( DuckCopyPublicHeaders PROPERTIES FOLDER Duck )
endif()


# Build Libraries ========================================================================
function( add_duck_library TargetName TargetType FRAMEWORK LibraryName )
    add_library( ${TargetName} ${TargetType}
        ${DuckSources}
        ${DuckICU}
        ${DuckYXML} )

    set_target_properties( ${TargetName} PROPERTIES
        OUTPUT_NAME ${LibraryName}
        ARCHIVE_OUTPUT_DIRECTORY ${DUCK_BINARY_PATH}
        LIBRARY_OUTPUT_DIRECTORY ${DUCK_BINARY_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${DUCK_BINARY_PATH}
        FOLDER Duck
        LINKER_LANGUAGE C )

    target_compile_definitions( ${TargetName}
         PUBLIC
         $<$<CONFIG:Debug>:DEBUG=1> )

    if( WIN32 )
        target_compile_definitions( ${TargetName}
            PUBLIC
            $<IF:$<STREQUAL:${TargetType},STATIC>,DK_API_STATIC,DK_API_EXPORTS>
            _CRT_SECURE_NO_WARNINGS )

        target_compile_options( ${TargetName} PUBLIC /EHsc )
    endif()

    if( FRAMEWORK )
        set_target_properties( ${TargetName} PROPERTIES
            FRAMEWORK 1 
            MACOSX_FRAMEWORK_INFO_PLIST ${DUCK_PATH}/Info.plist )
    endif()

    add_target_headers( ${TargetName}
        PUBLIC ${DuckSources}
        PROJECT ${DuckICU} ${DuckYXML} )

    target_include_directories( ${TargetName}
        PUBLIC ${DuckPublicHeaderSearchPaths}
        PRIVATE ${DuckProjectHeaderSearchPaths} )

    target_link_libraries( ${TargetName} ${DuckRequiredLibraries} )

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
if( STATIC IN_LIST DUCK_BUILD )
    add_duck_library( DuckStaticLibrary STATIC NO ${DuckStaticLibraryName} )
endif()


# Shared Library -------------------------------------------------------------------------
if( SHARED IN_LIST DUCK_BUILD )
    add_duck_library( DuckSharedLibrary SHARED NO ${DuckSharedLibraryName} )
endif()


# Framework ------------------------------------------------------------------------------
if( APPLE AND FRAMEWORK IN_LIST DUCK_BUILD )
    add_duck_library( DuckFramework SHARED YES ${DuckFrameworkName} )
endif()


# Build Examples =========================================================================
if( DUCK_BUILD_EXAMPLES )
    include( ${DUCK_PATH}/HelloWorld/CMakeLists.txt )
endif()




