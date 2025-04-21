@echo off
echo ==============================================
echo [BUILDER]: Building everything...

REM Ensure build directory exists
if not exist ../bin (
    mkdir ../bin
)

REM Run CMake configuration (assumes you're in project root)
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++ -B../bin . -D CMAKE_BUILD_TYPE=Debug

REM Build the project
cmake --build ../bin
if errorlevel 1 (
    echo [BUILDER]: Build failed with error %errorlevel%
    exit /b %errorlevel%
)

echo [BUILDER]: All assemblies built successfully.
echo.
echo [BUILDER]: Launching testbed...
echo ==============================================

REM Copy engine DLL to testbed's output directory
copy /Y "..\bin\engine\Debug\koala_engine.dll" "..\bin\testbed\Debug\"

cd ../bin/testbed/Debug/
start "" "testbed.exe"


