@REM cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" && set' | ForEach-Object {
@REM    $parts = $_ -split '=', 2
@REM    if ($parts.Length -eq 2) {
@REM       Set-Item -Path "Env:$($parts[0])" -Value $parts[1]
@REM    }
@REM }
@REM Write-Host "[PowerShell] MSVC environment loaded ✅"

@echo off
setlocal

set BUILD_DIR=..\bin
set CONFIG=Debug

echo ==============================================
echo [BUILDER]: Building everything with cl + Ninja...

REM Ensure clean build directory
if exist %BUILD_DIR% rmdir /S /Q %BUILD_DIR%
mkdir %BUILD_DIR%

REM Configure with cl
cmake -G "Ninja" ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_BUILD_TYPE=%CONFIG% ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -B %BUILD_DIR% .

if errorlevel 1 (
    echo [BUILDER]: CMake configuration failed!
    exit /b %errorlevel%
)

REM Build project
cd %BUILD_DIR%
@REM cmake --build %BUILD_DIR%
ninja
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

cd %BUILD_DIR%\testbed
start "" "testbed.exe"

