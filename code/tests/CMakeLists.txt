cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE TESTS_SRC "src/*.cpp")

# Add test executable
add_executable(koala_tests ${TESTS_SRC})

# Link with engine
target_link_libraries(koala_tests PRIVATE koala_engine)

enable_testing()
add_test(NAME KoalaTests COMMAND koala_tests)
