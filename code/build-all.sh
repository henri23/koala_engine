#!bin/bash

start_time=$(date +%s%3N)
echo "=============================================="
echo "[BUILDER]: Building everything..."

mkdir -p ../bin

# Run cmake to configure the project
time cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_CXX_COMPILER=clang++ -B../bin . -D CMAKE_BUILD_TYPE=Debug

cd ../bin
ln -sf "$(pwd)/compile_commands.json" ../code/compile_commands.json
# Build the project using make

time make

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "[BUILDER]: Error:"$ERRORLEVEL && exit
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

