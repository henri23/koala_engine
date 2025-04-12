cmake_minimum_required(VERSION 3.16)
project(koala_engine)

set(CMAKE_CXX_STANDARD 23)

# set(ENABLE_ASAN FALSE CACHE BOOL "Enable special configuration block")

# if(ENABLE_ASAN)
#     if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#         set(SANITIZER_FLAGS "-fsanitize=address")
#         add_compile_options(${SANITIZER_FLAGS})
#         add_link_options(${SANITIZER_FLAGS})
#     endif()
# endif()

add_subdirectory(engine)
add_subdirectory(testbed)
