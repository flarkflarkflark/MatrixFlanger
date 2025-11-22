#!/bin/bash

# flark's MatrixFilter - Universal Build Script
# Builds all plugin formats: CLAP, VST3, and LV2

set -e

echo "🚀 Building Complete Plugin Suite: flark's MatrixFilter"
echo "===================================================="
echo "🎵 Formats: CLAP (modern), VST3 (standard), LV2 (Linux-focused)"
echo "🖥️ Platforms: Linux, Windows, macOS"
echo "🎨 Features: 7 audio filters + real-time matrix visualization"
echo ""

# Create main build directory
mkdir -p build/suite
cd build/suite

# Check for required dependencies
echo "🔍 Checking dependencies..."

check_dependency() {
    if command -v $1 >/dev/null 2>&1; then
        echo "✅ $1 found"
        return 0
    else
        echo "❌ $1 not found"
        return 1
    fi
}

# Essential build tools
has_cmake=$(check_dependency cmake)
has_pkg_config=$(check_dependency pkg-config)
has_make=$(check_dependency make)

# Format-specific checks
has_clap=false
has_vst3=false
has_lv2=false

if pkg-config --exists clap 2>/dev/null; then
    echo "✅ CLAP SDK found"
    has_clap=true
else
    echo "⚠️  CLAP SDK not found (install: libclap-dev or brew install clap)"
fi

if pkg-config --exists vst3sdk 2>/dev/null; then
    echo "✅ VST3 SDK found"
    has_vst3=true
else
    echo "⚠️  VST3 SDK not found (install from Steinberg website)"
fi

if pkg-config --exists lv2 2>/dev/null; then
    echo "✅ LV2 SDK found"
    has_lv2=true
else
    echo "⚠️  LV2 SDK not found (install: lv2-dev or brew install lv2)"
fi

echo ""

# Build options
BUILD_CLAP=${BUILD_CLAP:-true}
BUILD_VST3=${BUILD_VST3:-true}
BUILD_LV2=${BUILD_LV2:-true}

# Auto-disable formats that don't have SDKs
if [ "$has_clap" = false ]; then BUILD_CLAP=false; fi
if [ "$has_vst3" = false ]; then BUILD_VST3=false; fi
if [ "$has_lv2" = false ]; then BUILD_LV2=false; fi

echo "📋 Build Configuration:"
echo "   CLAP:  $([ "$BUILD_CLAP" = true ] && echo "✅ YES" || echo "❌ NO")"
echo "   VST3:  $([ "$BUILD_VST3" = true ] && echo "✅ YES" || echo "❌ NO")"
echo "   LV2:   $([ "$BUILD_LV2" = true ] && echo "✅ YES" || echo "❌ NO")"
echo ""

# Build function for each format
build_format() {
    local format=$1
    local script=$2
    local output_dir=$3
    
    echo "🏗️  Building $format..."
    echo "======================"
    
    if [ -f "../../build-$format.sh" ]; then
        # Run format-specific build script
        if bash ../../build-$format.sh > "build_$format.log" 2>&1; then
            echo "✅ $format build completed successfully"
            return 0
        else
            echo "❌ $format build failed!"
            echo "Check build_$format.log for details"
            return 1
        fi
    else
        echo "⚠️  $format build script not found"
        return 1
    fi
}

# Build all enabled formats
success_count=0
total_count=0

if [ "$BUILD_CLAP" = true ]; then
    total_count=$((total_count + 1))
    if build_format "CLAP" "clap" "clap"; then
        success_count=$((success_count + 1))
    fi
    echo ""
fi

if [ "$BUILD_VST3" = true ]; then
    total_count=$((total_count + 1))
    if build_format "VST3" "vst3" "vst3"; then
        success_count=$((success_count + 1))
    fi
    echo ""
fi

if [ "$BUILD_LV2" = true ]; then
    total_count=$((total_count + 1))
    if build_format "LV2" "lv2" "lv2"; then
        success_count=$((success_count + 1))
    fi
    echo ""
fi

# Create unified installation directory
echo "📦 Creating unified installation package..."
mkdir -p install/flark-matrixfilter-suite

# Collect built plugins
for format in clap vst3 lv2; do
    if [ -d "$format/install" ]; then
        echo "📁 Collecting $format plugins..."
        cp -r $format/install/* install/flark-matrixfilter-suite/ 2>/dev/null || true
    fi
done

# Create distribution structure
echo "🌐 Creating distribution structure..."
cd install

# Organize by platform and format
mkdir -p flark-matrixfilter-suite/{linux,windows,macos,generic}

# Copy CLAP plugins
if [ -d "clap-audio-filter" ]; then
    echo "📁 Packaging CLAP plugins..."
    cp -r clap-audio-filter flark-matrixfilter-suite/generic/
fi

# Copy VST3 plugins
if [ -d "vst3" ]; then
    echo "📁 Packaging VST3 plugins..."
    cp -r vst3 flark-matrixfilter-suite/generic/
fi

# Copy LV2 plugins
if [ -d "lv2" ]; then
    echo "📁 Packaging LV2 plugins..."
    cp -r lv2 flark-matrixfilter-suite/generic/
fi

cd ../..

# Generate build report
echo "📊 Build Summary"
echo "==============="
echo "Formats built: $success_count/$total_count"

if [ $success_count -eq $total_count ] && [ $total_count -gt 0 ]; then
    echo "🎉 ALL FORMATS BUILT SUCCESSFULLY!"
    exit_code=0
else
    echo "⚠️  Some formats failed to build"
    if [ $total_count -eq 0 ]; then
        echo "❌ No formats could be built (missing SDKs)"
        exit_code=1
    else
        exit_code=1
    fi
fi

echo ""
echo "📁 Installation Packages:"
echo "   CLAP:  build/suite/install/flark-matrixfilter-suite/generic/clap-audio-filter/"
echo "   VST3:  build/suite/install/flark-matrixfilter-suite/generic/vst3/"
echo "   LV2:   build/suite/install/flark-matrixfilter-suite/generic/lv2/"
echo ""

echo "🎯 Next Steps:"
echo "1. Copy plugin files to your DAW's plugin directories"
echo "2. Check the comprehensive documentation in docs/"
echo "3. Test the matrix visualization with your audio sources"
echo ""

echo "📚 Documentation:"
echo "   README.md         - Complete plugin documentation"
echo "   QUICKSTART.md     - 5-minute setup guide"
echo "   FORMATS.md        - Format comparison and selection"
echo "   INSTALLATION.md   - Platform-specific installation"
echo ""

echo "🎵 flark's MatrixFilter Plugin Suite"
echo "===================================="
echo "✅ Professional audio filtering (7 filter types)"
echo "✅ Real-time matrix visual effects (OpenGL)"
echo "✅ Multi-format support (CLAP + VST3 + LV2)"
echo "✅ Cross-platform compatibility"
echo "✅ Zero-latency audio processing"
echo "✅ Professional DAW integration"
echo ""

exit $exit_code