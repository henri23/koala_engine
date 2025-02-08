cmake_minimum_required(VERSION 3.16)
project(testbed)

set(CMAKE_CXX_STANDARD 23)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE TESTBED_SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${TESTBED_SRC})

link_directories(${CMAKE_SOURCE_DIR}/koala_engine)

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan koala_engine)
# target_include_directories(${PROJECT_NAME} PRIVATE "engine/src")

