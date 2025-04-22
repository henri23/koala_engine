@echo off
echo ==============================================
echo [BUILDER]: Building everything...

set BUILD_DIR=..\bin-vs
set CONFIG=Debug

REM Ensure build directory exists
if not exist ../bin-vs (
    mkdir ../bin-vs
)

REM Run CMake configuration (assumes you're in project root)
cmake ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_BUILD_TYPE=%CONFIG% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -B %BUILD_DIR% .

@REM REM Link compile_commands.json (only works with symlinks enabled)
@REM if exist code\compile_commands.json del code\compile_commands.json
@REM mklink code\compile_commands.json bin-vs\compile_commands.json

REM Build the project
cmake --build ../bin-vs
if errorlevel 1 (
    echo [BUILDER]: Build failed with error %errorlevel%
    exit /b %errorlevel%
)

echo [BUILDER]: All assemblies built successfully.
echo.
echo [BUILDER]: Launching testbed...
echo ==============================================

REM Copy engine DLL to testbed's output directory
copy /Y "..\bin-vs\engine\Debug\koala_engine.dll" "..\bin-vs\testbed\Debug\"

cd ../bin-vs/testbed/Debug/
start "" "testbed.exe"

