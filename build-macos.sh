#!/bin/bash

# flark's MatrixFilter - macOS Build Script
# This script builds the plugin for macOS systems

set -e

echo "Building flark's MatrixFilter for macOS..."

# Create build directory
mkdir -p build/macos
cd build/macos

# Run CMake configuration
cmake ../.. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX=./install

echo "CMake configuration completed."

# Build the plugin
make -j$(sysctl -n hw.ncpu)

echo "Plugin build completed successfully."

# Create installation directory
mkdir -p install/MatrixFilter

# Copy plugin to installation directory (macOS uses .bundle extension)
cp libclap-audio-filter.bundle install/MatrixFilter/flark-MatrixFilter.clap

echo "Plugin copied to installation directory."

# Install CLAP libraries if needed
echo "Checking for CLAP library..."
if ! pkg-config --exists clap; then
    echo "CLAP library not found. Installing..."
    # Try Homebrew first
    if command -v brew >/dev/null 2>&1; then
        brew install clap
    else
        echo "Homebrew not found. Please install CLAP manually."
        echo "Visit: https://github.com/free-audio/clap"
        echo "Or install via Homebrew: /bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    fi
fi

echo "macOS build completed!"
echo "Plugin location: build/macos/install/MatrixFilter/flark-MatrixFilter.clap"
echo ""
echo "To use this plugin:"
echo "1. Copy flark-MatrixFilter.clap to your DAW's CLAP plugin directory"
echo "2. Typical locations:"
echo "   - ~/Library/Audio/Plug-Ins/CLAP/"
echo "   - /Library/Audio/Plug-Ins/CLAP/ (system-wide)"
echo "3. Rescan plugins in your DAW"
echo ""
echo "Note: macOS CLAP plugins use .bundle format internally but appear as .clap files"
echo ""
echo "Tested DAWs on macOS:"
echo "- Bitwig Studio (native CLAP support)"
echo "- Reaper (with CLAP plugin)"
echo "- Ardour (full CLAP compatibility)"