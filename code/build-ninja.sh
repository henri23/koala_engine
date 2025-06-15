#!/bin/bash

start_time=$(date +%s%3N)

echo "=============================================="
echo "[BUILDER]: Building everything..."

# Ensure the bin directory exists
mkdir -p ../bin

# Run CMake with Ninja generator and Clang++
time cmake -G Ninja \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Debug \
	-DKOALA_BUILD_TESTS=ON \
    -B ../bin \
    .

# Go into the bin directory
cd ../bin

# Link compile_commands.json for IDE integration
ln -sf "$(pwd)/compile_commands.json" ../code/compile_commands.json

# Build with Ninja
echo "[BUILDER]: Building tests..."
time ninja koala_tests

echo "[BUILDER]: Running tests..."
./tests/koala_tests
TEST_EXIT_CODE=$?

if [ $TEST_EXIT_CODE -ne 0 ]; then
    echo "[BUILDER]: Tests failed. Aborting build."
    exit 1
fi

echo "[BUILDER]: Building testbed..."
time ninja testbed  # Replace with actual testbed target

# Check for errors
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]; then
    echo "[BUILDER]: Error: $ERRORLEVEL" && exit
fi

# Record end time and calculate duration
end_time=$(date +%s%3N)
tottime=$(expr $end_time - $start_time)

# Print total elapsed time in seconds.milliseconds
echo "[BUILDER]: Total build time: $tottime ms\n"


echo "[BUILDER]: All assemblies built successfully."
echo
echo "[BUILDER]: Launching testbed..."
echo "=============================================="
echo ""

cd testbed
./testbed

