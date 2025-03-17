cmake_minimum_required(VERSION 3.16)
project(koala_engine)

set(CMAKE_CXX_STANDARD 23)

# Define project MACROS
add_compile_definitions(KOALA_EXPORT)

# Include all files recursivelly (header files do not need to be included)
file(GLOB_RECURSE ENGINE_SRC "src/*.cpp")

# Link with external libraries
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} SHARED ${ENGINE_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_include_directories(${PROJECT_NAME} PUBLIC "src")

# Check if the platform is Linux
if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE xcb xcb-util)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "koala_engine")

