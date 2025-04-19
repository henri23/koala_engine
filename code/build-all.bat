@echo off
echo ==============================================
echo [BUILDER]: Building everything...

REM Ensure build directory exists
if not exist bin (
    mkdir bin
)

REM Run CMake configuration (assumes you're in project root)
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=YES -DCMAKE_CXX_COMPILER=clang++ -B bin -D CMAKE_BUILD_TYPE=Debug

REM Link compile_commands.json (only works with symlinks enabled)
if exist code\compile_commands.json del code\compile_commands.json
mklink code\compile_commands.json bin\compile_commands.json

REM Build the project
cmake --build bin
if errorlevel 1 (
    echo [BUILDER]: Build failed with error %errorlevel%
    exit /b %errorlevel%
)

echo [BUILDER]: All assemblies built successfully.
echo.
echo [BUILDER]: Launching testbed...
echo ==============================================

cd bin\testbed
testbed.exe

