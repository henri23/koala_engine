cmake_minimum_required(VERSION 3.16)
project(testbed)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(DEBUG_BUILD)
file(GLOB_RECURSE TESTBED_SRC "src/*.cpp")

add_executable(${PROJECT_NAME} ${TESTBED_SRC})

link_directories(${CMAKE_SOURCE_DIR}/koala_engine)

target_link_libraries(${PROJECT_NAME} PRIVATE koala_engine)
