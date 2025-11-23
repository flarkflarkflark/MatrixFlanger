# flark's MatrixFilter Plugin Suite

## 🎵 Multi-Format Audio Plugin Suite

**flark's MatrixFilter** is a professional-grade audio filter plugin available in **two industry-standard formats** with stunning real-time matrix-style visual effects.

### 🎯 **Available Formats**

| Format | Description | Build Command |
|--------|-------------|---------------|
| **VST3** | Industry standard plugin format | `./build-vst3.sh` |
| **LV2** | Linux-focused, open-source format | `./build-lv2.sh` |

### 🚀 **Quick Start**

#### **Build All Formats** (Recommended)
```bash
chmod +x build-all.sh
./build-all.sh
```

#### **Platform-Specific Builds**
```bash
# Linux
chmod +x build-linux.sh && ./build-linux.sh

# Windows  
build-windows.bat

# macOS
chmod +x build-macos.sh && ./build-macos.sh
```

### 📚 **Complete Documentation**

- **[BUILD.md](BUILD.md)** - Comprehensive build guide for source releases
- **[FORMATS.md](FORMATS.md)** - Plugin format comparison and selection guide

### ✨ **Key Features**

#### **Audio Processing**
- **7 Professional Filters**: Low-pass, High-pass, Band-pass, Notch, Peaking, Low Shelf, High Shelf
- **Zero-Latency Processing**: Real-time audio filtering
- **Stereo Support**: Full stereo audio capabilities
- **Parameter Automation**: Complete automation support

#### **Visual Effects** 
- **Real-time Matrix Visualization**: Cascading characters responding to audio
- **OpenGL Rendering**: Hardware-accelerated graphics
- **Audio-Spectrum Analysis**: Live frequency domain visualization
- **Matrix Aesthetic**: Classic green-on-black theme

#### **Multi-Platform Support**
- **Windows**: VST3, LV2
- **Linux**: VST3, LV2 (native LV2 support)
- **macOS**: VST3, LV2

### 🎮 **Plugin Controls**

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Cutoff Frequency** | 20 Hz - 20 kHz | 1 kHz | Filter cutoff/center frequency |
| **Resonance** | 0.1 - 10.0 | 1.0 | Filter Q factor/resonance |
| **Gain** | -60 dB - +60 dB | 0 dB | Filter gain (for peaking/shelf filters) |
| **Filter Type** | 0 - 6 | 0 | Select filter type (7 different types) |
| **Enabled** | On/Off | On | Enable/disable the filter |

### 🎵 **DAW Compatibility**

#### **VST3 Format**  
- **Cubase** (native support) ⭐
- **Logic Pro** (with VST3 bridge) ⭐
- **Studio One** (native support) ⭐
- **Reaper** (native support) ⭐

#### **LV2 Format**
- **Ardour** (native support) ⭐
- **Hydrogen** (drum machine) ⭐
- **Carla** (universal host) ⭐
- **LMMS** (LV2 support) ⭐

### 📁 **Project Structure**

```
flark-matrixfilter/
├── 📄 README.md          # Main documentation
├── 📄 BUILD.md           # Build guide
├── 📄 FORMATS.md         # Format comparison guide
├── 📄 build-all.sh       # Universal build script
├── 📄 build-vst3.sh      # VST3 build script
├── 📄 build-lv2.sh       # LV2 build script
├── 📄 build-linux.sh     # Linux build script
├── 📄 build-windows.bat  # Windows build script
├── 📄 build-macos.sh     # macOS build script
├── 📁 vst3/              # VST3 format implementation
├── 📁 lv2/               # LV2 format implementation
├── 📁 src/               # Shared DSP and GUI code
└── 📁 include/           # Shared headers
```

### 🛠️ **System Requirements**

#### **Build Requirements**
- **CMake** 3.17+
- **C++17** compatible compiler
- **Platform-specific SDKs**:
  - VST3 SDK (Steinberg)
  - LV2 SDK (lv2-dev)

#### **Runtime Requirements**  
- **OpenGL 3.0+** graphics support
- **Modern audio interface** (ASIO/ALSA/CoreAudio)
- **1GB RAM** minimum, 2GB recommended
- **Any DAW** with format support

### 📊 **Format Selection Guide**

**Choose VST3 if you want:**
- Maximum DAW compatibility
- Industry-standard format
- Professional production environment
- Extensive plugin ecosystem

**Choose LV2 if you want:**
- Linux-first approach
- Fully open-source audio software
- Ardour, Hydrogen, or Carla workflow
- Advanced customization capabilities

### 🎨 **Matrix Visualization Features**

- **Real-time Effects**: OpenGL-powered graphics
- **Audio Responsive**: Visual effects react to input audio
- **Cascading Characters**: Classic Matrix rain animation
- **Spectrum Analysis**: Live frequency visualization  
- **60 FPS Performance**: Smooth, professional-grade rendering
- **Hardware Acceleration**: GPU-optimized implementation

### 🔧 **Development**

#### **Building Individual Formats**
```bash
# VST3 format
./build-vst3.sh

# LV2 format
./build-lv2.sh
```

#### **Custom Builds**
```bash
# Build specific formats only
BUILD_VST3=true BUILD_LV2=false ./build-all.sh

# Platform optimization
./build-linux.sh   # Linux optimized
./build-macos.sh   # macOS optimized
```

### 🐛 **Troubleshooting**

**Common Issues:**

1. **Plugin not appearing in DAW**
   - Verify correct format for your DAW
   - Check file permissions (Linux/macOS)
   - Rescan plugins in your DAW

2. **Matrix visualization not displaying**
   - Update graphics drivers
   - Check OpenGL support: `glxinfo | grep OpenGL`
   - Ensure sufficient GPU memory

3. **Audio artifacts**
   - Increase DAW buffer size
   - Check audio driver performance
   - Verify sufficient CPU resources

### 🏆 **What's Included**

✅ **Professional Audio Processing**
- 7 filter types with accurate frequency response
- Zero-latency real-time processing  
- Complete stereo support
- Parameter automation support

✅ **Stunning Visual Effects**
- Real-time matrix-style visualization
- Audio-responsive cascading effects
- Hardware-accelerated OpenGL rendering
- 60 FPS smooth animation

✅ **Multi-Format Support**
- VST3 (industry standard)
- LV2 (Linux-focused, open-source)
- Cross-platform compatibility

✅ **Professional Integration**
- Compatible with all major DAWs
- Thread-safe implementation
- Memory efficient design
- Professional-grade stability

### 📄 **License**

This project is open source. See individual format directories for specific license terms.

### 👨‍💻 **Credits**

**Developed by flark**

- **VST3 SDK**: Steinberg (Proprietary)
- **LV2 Standard**: http://lv2plug.in/ (ISC)
- **Matrix Effects**: Inspired by "The Matrix" movie
- **Audio DSP**: Standard biquad filter algorithms
- **Build System**: CMake cross-platform build

### 🎉 **Ready to Build?**

Start with the complete suite:
```bash
chmod +x build-all.sh
./build-all.sh
```

Or jump to specific documentation:
- **[Build Guide](BUILD.md)** - Complete build instructions
- **[Format Comparison](FORMATS.md)** - Choose your optimal format

---

**Version**: 1.0.0 | **Formats**: VST3, LV2 | **Platforms**: Windows, Linux, macOS | **Author**: flark