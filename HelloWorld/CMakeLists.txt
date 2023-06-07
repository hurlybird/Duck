# Duck - HelloWorld Example

# Create the HelloWorld target
add_executable( HelloWorld ${DUCK_PATH}/HelloWorld/HelloWorld.c )

if( APPLE AND NOT IOS )
    add_target_property( HelloWorld OSX_ARCHITECTURES "arm64;x86_64" )
endif()

# Link against the Duck Library
target_include_directories( HelloWorld PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )

if( STATIC IN_LIST DUCK_BUILD )
    target_link_libraries( HelloWorld DuckStaticLibrary )
elseif( SHARED IN_LIST DUCK_BUILD )
    target_link_libraries( HelloWorld DuckSharedLibrary )
elseif( APPLE AND (FRAMEWORK IN_LIST DUCK_BUILD) )
    target_link_libraries( HelloWorld DuckFramework )
endif()
