@echo off

REM flark's MatrixFlanger - Windows Build Script
REM Builds VST3 and LV2 plugins for Windows systems

echo ====================================================
echo flark's MatrixFlanger - Windows Build Script
echo ====================================================
echo Building VST3 and LV2 plugins for Windows
echo.

REM Create main build directory
if not exist build\windows mkdir build\windows
cd build\windows

REM Check for required dependencies
echo Checking dependencies...
echo.

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] CMake found
) else (
    echo [ERROR] CMake not found
    echo Please install CMake from https://cmake.org/
    exit /b 1
)

REM Check for pkg-config
where pkg-config >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] pkg-config found
) else (
    echo [WARNING] pkg-config not found
)

REM Check for SDKs
set has_vst3=0
set has_lv2=0

pkg-config --exists vst3sdk >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] VST3 SDK found
    set has_vst3=1
) else (
    echo [WARNING] VST3 SDK not found
    echo Install from https://www.steinberg.net/vst3sdk
)

pkg-config --exists lv2 >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [OK] LV2 SDK found
    set has_lv2=1
) else (
    echo [WARNING] LV2 SDK not found
    echo Install from http://lv2plug.in/
)

echo.
echo Build Configuration:
if %has_vst3%==1 (echo    VST3:  YES) else (echo    VST3:  NO)
if %has_lv2%==1 (echo    LV2:   YES) else (echo    LV2:   NO)
echo.

REM Build VST3 if SDK is available
if %has_vst3%==1 (
    echo ====================================================
    echo Building VST3...
    echo ====================================================

    if not exist vst3 mkdir vst3
    cd vst3

    cmake ..\..\vst3 ^
        -G "Visual Studio 16 2019" ^
        -A x64 ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_INSTALL_PREFIX=.\install

    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] VST3 CMake configuration failed
        cd ..
        goto build_lv2
    )

    cmake --build . --config Release

    if %ERRORLEVEL% EQU 0 (
        echo [OK] VST3 build completed successfully
    ) else (
        echo [ERROR] VST3 build failed
    )

    cd ..
    echo.
)

:build_lv2
REM Build LV2 if SDK is available
if %has_lv2%==1 (
    echo ====================================================
    echo Building LV2...
    echo ====================================================

    if not exist lv2 mkdir lv2
    cd lv2

    cmake ..\..\lv2 ^
        -G "Visual Studio 16 2019" ^
        -A x64 ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_INSTALL_PREFIX=.\install

    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] LV2 CMake configuration failed
        cd ..
        goto build_summary
    )

    cmake --build . --config Release

    if %ERRORLEVEL% EQU 0 (
        echo [OK] LV2 build completed successfully
    ) else (
        echo [ERROR] LV2 build failed
    )

    cd ..
    echo.
)

:build_summary
REM Create unified installation package
echo ====================================================
echo Creating installation package...
echo ====================================================

if not exist install\flark-matrixflanger-suite mkdir install\flark-matrixflanger-suite

REM Collect VST3 plugins
if exist vst3\install (
    echo Collecting VST3 plugins...
    xcopy /E /I /Y vst3\install\* install\flark-matrixflanger-suite\
)

REM Collect LV2 plugins
if exist lv2\install (
    echo Collecting LV2 plugins...
    xcopy /E /I /Y lv2\install\* install\flark-matrixflanger-suite\
)

echo.
echo ====================================================
echo Build Summary
echo ====================================================

if %has_vst3%==0 if %has_lv2%==0 (
    echo [ERROR] No formats could be built - missing SDKs
    echo.
    echo Please install:
    echo - VST3 SDK: https://www.steinberg.net/vst3sdk
    echo - LV2 SDK: http://lv2plug.in/
    cd ..\..
    pause
    exit /b 1
)

echo Build completed!
echo.
echo Installation Packages:
if %has_vst3%==1 echo    VST3:  build\windows\install\flark-matrixflanger-suite\vst3\
if %has_lv2%==1 echo    LV2:   build\windows\install\flark-matrixflanger-suite\lv2\
echo.

echo ====================================================
echo Installation Instructions
echo ====================================================
echo.
echo VST3 Plugin:
echo 1. Copy .vst3 file to: C:\Program Files\Common Files\VST3\
echo 2. Or to: C:\Users\%%USERNAME%%\AppData\Local\Programs\Common\VST3\
echo.
echo LV2 Plugin:
echo 1. Copy .lv2 bundle to: C:\Program Files\Common Files\LV2\
echo 2. Or to: %%APPDATA%%\LV2\
echo.

echo Compatible DAWs:
echo VST3: Cubase, Studio One, Reaper, FL Studio, Ableton Live
echo LV2:  Ardour, Qtractor, Carla
echo.

echo Documentation:
echo    README.md  - Complete plugin documentation
echo    BUILD.md   - Comprehensive build guide
echo    FORMATS.md - Format comparison guide
echo.

cd ..\..
pause
