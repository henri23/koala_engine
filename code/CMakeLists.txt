cmake_minimum_required(VERSION 3.16)
project(koala_engine)

set(CMAKE_CXX_STANDARD 23)

# Build engine library
add_subdirectory(engine)

# Could just run tests separatelly inside bin with ctest but i prefer to include
# test in my build script
option(KOALA_BUILD_TESTS "Build Koala unit tests" ON)
# IF (KOALA_BUILD_TESTS)
	# enable_testing()
	add_subdirectory(tests)
# ENDIF()

# Build testbed app
add_subdirectory(testbed)
