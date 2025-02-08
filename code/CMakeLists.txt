cmake_minimum_required(VERSION 3.16)
project(koala_engine)

set(CMAKE_CXX_STANDARD 23)
find_package(Vulkan REQUIRED)

# add_subdirectory(testbed)
# add_subdirectory(engine)
add_executable(${PROJECT_NAME} "testbed/src/main.cpp")

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
