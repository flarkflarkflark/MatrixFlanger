#!/bin/bash

# flark's MatrixFlanger - Linux Build Script
# Builds VST3 and LV2 plugins for Linux systems

set -e

echo "🚀 Building flark's MatrixFlanger for Linux..."
echo "================================================"
echo ""

# Check for required tools
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Install with: sudo apt-get install cmake"
    exit 1
fi

if ! command -v pkg-config &> /dev/null; then
    echo "❌ pkg-config not found. Install with: sudo apt-get install pkg-config"
    exit 1
fi

# Check for SDKs
echo "🔍 Checking for required SDKs..."
has_vst3=false
has_lv2=false

if pkg-config --exists vst3sdk 2>/dev/null; then
    echo "✅ VST3 SDK found"
    has_vst3=true
else
    echo "⚠️  VST3 SDK not found"
    echo "   Download from: https://www.steinberg.net/developers/"
fi

if pkg-config --exists lv2 2>/dev/null; then
    echo "✅ LV2 SDK found"
    has_lv2=true
else
    echo "⚠️  LV2 SDK not found"
    echo "   Install with: sudo apt-get install lv2-dev"
fi

if [ "$has_vst3" = false ] && [ "$has_lv2" = false ]; then
    echo ""
    echo "❌ No SDKs found. Please install at least one SDK to build."
    exit 1
fi

echo ""

# Build VST3
if [ "$has_vst3" = true ]; then
    echo "🏗️  Building VST3 plugin..."
    mkdir -p build/vst3
    cd build/vst3
    cmake ../../vst3 -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo "✅ VST3 build completed"
    cd ../..
    echo ""
fi

# Build LV2
if [ "$has_lv2" = true ]; then
    echo "🏗️  Building LV2 plugin..."
    mkdir -p build/lv2
    cd build/lv2
    cmake ../../lv2 -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo "✅ LV2 build completed"
    cd ../..
    echo ""
fi

echo "🎉 Linux build completed successfully!"
echo ""
echo "📦 Installation:"
if [ "$has_vst3" = true ]; then
    echo "VST3: cp build/vst3/*.vst3 ~/.vst3/"
fi
if [ "$has_lv2" = true ]; then
    echo "LV2:  cp -r build/lv2/*.lv2 ~/.lv2/"
fi
echo ""
echo "🎵 flark's MatrixFlanger - Professional Audio Filtering"
