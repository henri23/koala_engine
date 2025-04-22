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
    -B ../bin \
    .

# Go into the bin directory
cd ../bin

# Link compile_commands.json for IDE integration
ln -sf "$(pwd)/compile_commands.json" ../code/compile_commands.json

# Build with Ninja
time ninja

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

