:: Call vcvars64.bat to set up MSVC environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Now you're inside the MSVC environment
echo [BUILDER]: MSVC environment loaded 

@echo off
setlocal

set BUILD_DIR=..\bin
set CONFIG=Debug

echo ==============================================
echo [BUILDER]: Building everything with cl + Ninja...

REM Ensure clean build directory
:: Only create the build directory if it doesn't already exist
if not exist %BUILD_DIR% (
    echo [Builder] Creating build directory: %BUILD_DIR%
    mkdir %BUILD_DIR%
) else (
    echo [Builder] Build directory already exists: %BUILD_DIR%
)

REM Configure with cl
cmake -G "Ninja" ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_BUILD_TYPE=%CONFIG% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
	-DKOALA_BUILD_TESTS=ON ^
    -B %BUILD_DIR% .

if errorlevel 1 (
    echo [BUILDER]: CMake configuration failed!
    exit /b %errorlevel%
)

cd %BUILD_DIR%
ninja koala_tests 

copy /Y "%BUILD_DIR%\engine\koala_engine.dll" "%BUILD_DIR%\tests"

cd %BUILD_DIR%\tests
.\koala_tests.exe

if errorlevel 1 (
    echo [BUILDER]: Tests failed! Aborting build...
    exit /b %errorlevel%
)

REM Build project
cd ../ 
@REM cmake --build %BUILD_DIR%
ninja testbed

if errorlevel 1 (
    echo [BUILDER]: Build failed!
    exit /b %errorlevel%
)

REM Copy compile_commands.json to root
copy /Y "%BUILD_DIR%\compile_commands.json" compile_commands.json

REM Copy engine DLL
copy /Y "%BUILD_DIR%\engine\koala_engine.dll" "%BUILD_DIR%\testbed"

echo [BUILDER]: Build completed.
echo ==============================================

cd %BUILD_DIR%
.\testbed\testbed.exe

