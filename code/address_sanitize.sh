#!bin/bash
echo "=============================================="
echo "[BUILDER]: Building everything..."

# # Check if the 'build' directory exists
# if [ ! -d "bin" ]; then
#   # If the 'build' directory does not exist, create it
#   mkdir bin 
# fi
#
# # Change into the 'build' directory
# cd bin

# Run cmake to configure the project
cmake -DENABLE_ASAN=TRUE -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B../bin . -D CMAKE_BUILD_TYPE=Debug
# time cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B../bin . -D CMAKE_BUILD_TYPE=Debug

cd ../bin
ln -sf "$(pwd)/compile_commands.json" ../code/compile_commands.json
# Build the project using make


make

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "[BUILDER]: Error:"$ERRORLEVEL && exit
fi

echo "[BUILDER]: All assemblies built successfully."
echo
echo "[BUILDER]: Launching testbed..."
echo "=============================================="
echo ""
