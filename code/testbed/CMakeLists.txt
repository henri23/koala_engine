cmake_minimum_required(VERSION 3.16)
project(testbed)

set(CMAKE_CXX_STANDARD 23)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE TESTBED_SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${TESTBED_SRC})

link_directories(${CMAKE_SOURCE_DIR}/koala_engine)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan koala_engine)

# if(CMAKE_BUILD_TYPE MATCHES "Debug")
#   # set(
#   #   CMAKE_C_FLAGS
#   #   "${CMAKE_C_FLAGS} -Werror -fsanitize=undefined -fsanitize=address"
#   # )
#   target_link_options(${PROJECT_NAME}
#     BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address
#   )
# endif()
