#!/bin/bash

# flark's MatrixFilter - VST3 Build Script
# This script builds the VST3 plugin for all platforms

set -e

echo "Building flark's MatrixFilter VST3 Plugin..."

# Create build directory
mkdir -p build/vst3
cd build/vst3

# Check for VST3 SDK
if ! pkg-config --exists vst3sdk 2>/dev/null; then
    echo "VST3 SDK not found. Installing..."
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS: Check for Homebrew
        if command -v brew >/dev/null 2>&1; then
            brew install vst-sdk || echo "Please install VST3 SDK manually from https://www.steinberg.net/vst3sdk"
        else
            echo "Please install Homebrew first: /bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        fi
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux: Try to install vst-sdk or build from source
        sudo apt-get update
        sudo apt-get install -y cmake build-essential pkg-config
        if sudo apt-get install -y vst-sdk 2>/dev/null; then
            echo "VST3 SDK installed via package manager"
        else
            echo "Please install VST3 SDK manually from https://www.steinberg.net/vst3sdk"
            echo "Or build from source: https://github.com/steinbergmedia/vst3sdk"
        fi
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
        # Windows: Manual installation required
        echo "Please install VST3 SDK manually from https://www.steinberg.net/vst3sdk"
    fi
fi

# Run CMake configuration
echo "Running CMake configuration..."

if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS build
    cmake ../.. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_INSTALL_PREFIX=./install \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux build
    cmake ../.. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_INSTALL_PREFIX=./install
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    # Windows build
    cmake ../.. \
        -G "Visual Studio 16 2019" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=.\install
else
    # Generic Unix build
    cmake ../.. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=./install
fi

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    echo "Please ensure VST3 SDK is installed and pkg-config can find vst3sdk"
    exit 1
fi

echo "CMake configuration completed."

# Build the plugin
echo "Building VST3 plugin..."

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    # Windows: Use MSBuild
    cmake --build . --config Release
else
    # Unix/Linux/macOS: Use make
    make -j$(nproc || sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

if [ $? -ne 0 ]; then
    echo "Plugin build failed!"
    exit 1
fi

echo "VST3 plugin build completed successfully."

# Create installation directory
mkdir -p install/vst3

# Copy plugin to installation directory
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" || "$OSTYPE" == "win32" ]]; then
    # Windows
    if [ -f "Release/flark-matrixfilter-vst3.vst3" ]; then
        cp -r Release/flark-matrixfilter-vst3.vst3 install/vst3/flark-MatrixFilter.vst3
    elif [ -f "flark-matrixfilter-vst3/Contents/Win64/flark-matrixfilter-vst3.vst3" ]; then
        cp -r flark-matrixfilter-vst3 install/vst3/flark-MatrixFilter.vst3
    else
        echo "Warning: Could not find VST3 plugin file. Check build output."
    fi
else
    # Unix/Linux/macOS
    if [ -d "flark-matrixfilter-vst3.vst3" ]; then
        cp -r flark-matrixfilter-vst3.vst3 install/vst3/flark-MatrixFilter.vst3
    elif [ -f "libflark-matrixfilter-vst3.bundle/Contents/MacOS/flark-matrixfilter-vst3" ]; then
        cp -r libflark-matrixfilter-vst3.bundle install/vst3/flark-MatrixFilter.vst3
    else
        echo "Warning: Could not find VST3 plugin file. Check build output."
    fi
fi

echo "Plugin copied to installation directory."

echo ""
echo "VST3 build completed!"
echo "Plugin location: build/vst3/install/vst3/flark-MatrixFilter.vst3"
echo ""
echo "To use this VST3 plugin:"
echo "1. Copy flark-MatrixFilter.vst3 to your DAW's VST3 plugin directory"
echo "2. Typical locations:"
echo "   Linux:"
echo "     - ~/.vst3/"
echo "     - /usr/lib/vst3/"
echo "   macOS:"
echo "     - ~/Library/Audio/Plug-Ins/VST3/"
echo "     - /Library/Audio/Plug-Ins/VST3/"
echo "   Windows:"
echo "     - C:\Program Files\Common Files\VST3\"
echo "     - C:\Users\%USERNAME%\AppData\Local\VST3\"
echo ""
echo "Compatible DAWs with VST3 support:"
echo "- Cubase (native VST3)"
echo "- Nuendo (native VST3)"
echo "- Logic Pro (requires VST3 bridge)"
echo "- Studio One (native VST3)"
echo "- Reaper (native VST3)"
echo "- Pro Tools (with VST3 bridge)"
echo "- Ableton Live (with VST3 support)"
echo "- Any DAW with VST3 compatibility"
echo ""
echo "Note: VST3 plugins require proper VST3 SDK installation."
echo "The MatrixFlanger VST3 includes professional audio filtering with Matrix-themed visualization."