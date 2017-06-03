# Duck Object Library - Android Buiid

# Copy the following into your project's CMake file replacing
# 'PATH_TO_DUCK_LIBRARY' with the appropriate value.

# set( DUCK_PATH "PATH_TO_DUCK_LIBRARY" )
# include( "${DUCK_PATH}"/Duck/CMakeLists.txt"

file( GLOB DuckSources "${DUCK_PATH}/Duck/Source/*.c" )
file( GLOB DuckICU "${DUCK_PATH}/Duck/ThirdParty/icu/*.c" )
file( GLOB DuckYXML "${DUCK_PATH}/Duck/ThirdParty/yxml/*.c" )

add_library( duck STATIC
    ${DuckSources}
    ${DuckICU}
    ${DuckYXML} )

target_compile_definitions( duck PUBLIC DK_PLATFORM_ANDROID=1 )

target_include_directories( duck
    PUBLIC ${DUCK_PATH}
    PUBLIC "${DUCK_PATH}/Duck/Source"
    PRIVATE "${DUCK_PATH}/Duck/ThirdParty"
    PRIVATE "${DUCK_PATH}/Duck/ThirdParty/icu" )

