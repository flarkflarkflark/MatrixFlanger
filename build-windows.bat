@echo off

REM flark's MatrixFilter - Windows Build Script
REM This script builds the plugin for Windows systems

echo Building flark's MatrixFilter for Windows...

REM Create build directory
if not exist build\windows mkdir build\windows
cd build\windows

REM Run CMake configuration
cmake ..\.. -G "Visual Studio 16 2019" -A x64 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_INSTALL_PREFIX=.\install

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo CMake configuration completed.

REM Build the plugin
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Plugin build failed!
    exit /b 1
)

echo Plugin build completed successfully.

REM Create installation directory
if not exist install\MatrixFilter mkdir install\MatrixFilter

REM Copy plugin to installation directory
copy Release\MatrixFilter.dll install\MatrixFilter\flark-MatrixFilter.clap

echo Plugin copied to installation directory.

REM Check for CLAP library
echo Checking for CLAP library...
where pkg-config >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: pkg-config not found. Make sure CLAP is properly installed.
    echo You can download CLAP SDK from: https://github.com/free-audio/clap
)

echo.
echo Windows build completed!
echo Plugin location: build\windows\install\MatrixFilter\flark-MatrixFilter.clap
echo.
echo To use this plugin:
echo 1. Copy flark-MatrixFilter.clap to your DAW's CLAP plugin directory
echo 2. Typical locations:
echo    - C:\Program Files\Common Files\CLAP\
echo    - C:\Users\%%USERNAME%%\AppData\Local\CLAP\Plugins\
echo.
echo Supported DAWs with CLAP support:
echo - Bitwig Studio
echo - Reaper (with CLAP plugin)
echo - Any DAW with CLAP support
echo.
echo Note: This plugin requires OpenGL support and will work best with
echo modern graphics drivers that support OpenGL 3.0 or higher.

pause