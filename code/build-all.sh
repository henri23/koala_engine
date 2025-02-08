echo "Building everything..."

# Check if the 'build' directory exists
if [ ! -d "build" ]; then
  # If the 'build' directory does not exist, create it
  mkdir build
fi

# Change into the 'build' directory
cd build

# Run cmake to configure the project
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..

ln -sf "$(pwd)/compile_commands.json" ../compile_commands.json
# Build the project using make
make

ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "All assemblies built successfully."
