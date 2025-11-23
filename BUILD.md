# Building MatrixFilter from Source

This guide will help you build the MatrixFilter plugin suite from source code.

## Prerequisites

### Required Build Tools
- **CMake** 3.17 or later
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **pkg-config** (for dependency management)
- **Git** (for version control)

### Platform-Specific Compilers

**Linux:**
```bash
sudo apt-get install build-essential cmake pkg-config
```

**macOS:**
```bash
xcode-select --install
brew install cmake pkg-config
```

**Windows:**
- Visual Studio 2017 or later with C++ desktop development
- CMake for Windows
- vcpkg for package management (optional)

## SDK Requirements

### VST3 SDK

**Download:**
1. Visit https://www.steinberg.net/developers/
2. Download VST3 SDK
3. Extract to a known location

**Install (Linux/macOS):**
```bash
cd vst3sdk
mkdir build && cd build
cmake ..
sudo make install
```

**Install (Windows):**
```cmd
cd vst3sdk
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
cmake --install .
```

### LV2 SDK

**Linux:**
```bash
sudo apt-get install lv2-dev
```

**macOS:**
```bash
brew install lv2
```

**Windows:**
- Download from http://lv2plug.in/
- Follow installation instructions

## OpenGL Requirements

### Linux
```bash
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

### macOS
OpenGL is included with Xcode Command Line Tools

### Windows
OpenGL is typically included with graphics drivers

## Building the Plugins

### Quick Build (All Formats)

**Linux/macOS:**
```bash
cd MatrixFlanger
chmod +x build-all.sh
./build-all.sh
```

**Windows:**
```cmd
cd MatrixFlanger
build-windows.bat
```

### Build Individual Formats

**VST3 Only:**
```bash
chmod +x build-vst3.sh
./build-vst3.sh
```

**LV2 Only:**
```bash
chmod +x build-lv2.sh
./build-lv2.sh
```

### Manual CMake Build

**VST3:**
```bash
mkdir -p build/vst3
cd build/vst3
cmake ../../vst3
make
```

**LV2:**
```bash
mkdir -p build/lv2
cd build/lv2
cmake ../../lv2
make
```

## Build Output Locations

After successful build:

**VST3:**
- Linux: `build/vst3/flark-matrixfilter-vst3.vst3`
- macOS: `build/vst3/flark-MatrixFilter.vst3`
- Windows: `build/vst3/Release/flark-MatrixFilter.vst3`

**LV2:**
- All platforms: `build/lv2/flark-matrixfilter.lv2/`

## Installation

### VST3

**Linux:**
```bash
mkdir -p ~/.vst3
cp -r build/vst3/*.vst3 ~/.vst3/
```

**macOS:**
```bash
cp -r build/vst3/flark-MatrixFilter.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**Windows:**
```cmd
xcopy build\vst3\Release\*.vst3 "%CommonProgramFiles%\VST3\" /E /I
```

### LV2

**Linux:**
```bash
mkdir -p ~/.lv2
cp -r build/lv2/flark-matrixfilter.lv2 ~/.lv2/
```

**macOS:**
```bash
mkdir -p ~/Library/Audio/Plug-Ins/LV2
cp -r build/lv2/flark-matrixfilter.lv2 ~/Library/Audio/Plug-Ins/LV2/
```

## Troubleshooting

### "VST3 SDK not found"
Ensure VST3 SDK is installed and pkg-config can find it:
```bash
pkg-config --modversion vst3sdk
```

If not found, you may need to set PKG_CONFIG_PATH:
```bash
export PKG_CONFIG_PATH=/path/to/vst3sdk/build:$PKG_CONFIG_PATH
```

### "LV2 SDK not found"
Install lv2-dev package or set PKG_CONFIG_PATH:
```bash
# Linux
sudo apt-get install lv2-dev

# macOS
brew install lv2
```

### "OpenGL not found"
Install OpenGL development libraries:
```bash
# Linux
sudo apt-get install libgl1-mesa-dev

# macOS (usually pre-installed)
xcode-select --install

# Windows
# Update graphics drivers
```

### Compilation Errors

**Missing C++17 support:**
```bash
# Update compiler
# Linux: sudo apt-get install g++-7 or later
# macOS: xcode-select --install
# Windows: Install Visual Studio 2017 or later
```

**Linking errors:**
Ensure all required libraries are installed:
```bash
# Linux
sudo apt-get install libpthread-stubs0-dev

# Check for missing libraries
ldd build/vst3/flark-matrixfilter-vst3.so
```

## Testing the Build

### Verify VST3
```bash
# Linux/macOS
file build/vst3/*.vst3

# Should show: shared object or bundle
```

### Verify LV2
```bash
lv2ls | grep flark
# Should show: http://flark.dev/matrixfilter
```

## Development Build

For development with debugging symbols:
```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Clean Build

Remove all build artifacts:
```bash
rm -rf build/
./build-all.sh
```

## Cross-Compilation

### Linux to Windows (MinGW)
```bash
mkdir build-windows
cd build-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake
make
```

### macOS Universal Binary (x86_64 + arm64)
```bash
cmake -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
make
```

## Build Options

Customize build with CMake variables:

```bash
cmake \
  -DBUILD_VST3=ON \
  -DBUILD_LV2=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  ..
```

## Getting Help

If you encounter build issues:
1. Check that all SDKs are properly installed
2. Verify pkg-config can find the dependencies
3. Review the build log for specific errors
4. File an issue at https://github.com/flarkflarkflark/MatrixFlanger/issues

## License

This build system is part of the MatrixFilter Plugin Suite.
See LICENSE file for details.

---

**Happy Building!** 🎵🔧
