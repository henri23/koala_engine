cmake_minimum_required(VERSION 3.16)
project(koala_engine)

set(CMAKE_CXX_STANDARD 23)

# if(CMAKE_BUILD_TYPE STREQUAL "DebugASAN")
    message(STATUS "Building in Debug mode with Clang Address Sanitizer")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(SANITIZER_FLAGS "-fsanitize=address")
        add_compile_options(${SANITIZER_FLAGS})
        add_link_options(${SANITIZER_FLAGS})
    endif()
# endif()

add_subdirectory(engine)
add_subdirectory(testbed)
