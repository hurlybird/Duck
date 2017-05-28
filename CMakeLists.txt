
# Sample Android Build with CMake

# Copy the following into your project's CMake file and replace
# 'PATH_TO_DUCK_LIBRARY' below with the appropriate value.

set( DUCK_PATH "PATH_TO_DUCK_LIBRARY" )

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

