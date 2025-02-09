#!bin/bash
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
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -B../bin . -D CMAKE_BUILD_TYPE=Debug
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .. -D CMAKE_BUILD_TYPE=Debug

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
echo "[BUILDER]: Running testbed"

cd testbed
./testbed

