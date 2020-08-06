# Duck - HelloWorld Example

# Create the HelloWorld target
add_executable( HelloWorld ${DUCK_PATH}/HelloWorld/HelloWorld.c )

# Link against the Duck Library
target_include_directories( HelloWorld PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
target_link_libraries( HelloWorld DuckStaticLibrary )

