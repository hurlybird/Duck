
cmake_minimum_required( VERSION 3.0 )

project( Duck-HelloWorld )

# Configure and include the Duck library
get_filename_component( DUCK_PATH "../" ABSOLUTE )
set( DUCK_PUBLIC_HEADERS_PATH ${PROJECT_BINARY_DIR}/Include )

include( "${DUCK_PATH}/CMakeLists.txt" )

# Create the HelloWorld target
add_executable( HelloWorld HelloWorld.c )

# Link against the Duck Library
target_include_directories( HelloWorld PUBLIC ${DUCK_PUBLIC_HEADERS_PATH} )
target_link_libraries( HelloWorld duck-shared )

