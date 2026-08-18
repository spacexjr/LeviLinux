# LeviLinux

LeviLinux is a modular, native Linux launcher built from the integration of three specialized projects:

## Project Integration Overview

### 1. Core Engine (`src/core/`)
- **Adapted from**: Trinity-Launcher-main
- **Purpose**: Main engine with version management, game launching, and core functionality
- **Key Features**:
  - Platform abstraction layer for Linux
  - Native library loading and symbol resolution
  - Mod loading and management
  - Cross-architecture support (x86_64, aarch64)

### 2. Library Loader (`src/loader/`)
- **Adapted from**: LeviLaunchroid-main
- **Purpose**: Native .so loading using dlopen/dlsym with JNI compatibility
- **Key Features**:
  - Dynamic library loading with error handling
  - Symbol resolution for native functions
  - Mod loading and runtime injection
  - Thread-safe operations

### 3. UI Layer (`src/ui/`)
- **Adapted from**: LeviLauncher-main
- **Purpose**: Frontend interface and user interaction
- **Key Features**:
  - Native Linux windowing (X11/Wayland)
  - Game version selection and management
  - Content browsing and installation
  - Settings and configuration

## Key Technical Components

### Platform Abstractions
- **Graphics**: OpenGL/Vulkan with Android API shims for compatibility
- **Audio**: ALSA/PulseAudio/PipeWire support
- **Input**: Keyboard/mouse/gamepad handling
- **System**: Process management and resource handling

### Library Loading
- **Dynamic Loading**: `dlopen()`/`dlsym()` wrapper with error handling
- **Symbol Resolution**: Runtime function lookup
- **Thread Safety**: Mutex-protected operations
- **JNI Compatibility**: Android API compatibility layer

### Architecture Support
- **x86_64**: 64-bit Intel/AMD processors
- **aarch64**: 64-bit ARM processors
- **Cross-platform**: Builds for both architectures

## Build System

### Requirements
- CMake 3.17+
- GCC/Clang compiler
- Qt6 development libraries
- X11/Wayland libraries
- ALSA/PulseAudio/PipeWire libraries

### Build Commands
```bash
# Create build directory and configure
cd LeviLinux
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install (optional)
sudo make install
```

## Directory Structure

```
LeviLinux/
├── include/
│   ├── core/              # Core engine headers
│   │   ├── core_engine.hpp
│   │   └── platform_abstraction.hpp
│   ├── loader/           # Library loading headers
│   │   ├── library_loader.hpp
│   │   └── mod_loader.hpp
│   └── ui/               # UI layer headers (planned)
├── src/
│   ├── core/             # Core engine implementation
│   │   ├── core_engine.cpp
│   │   └── platform_abstraction.cpp
│   ├── loader/           # Library loading implementation
│   │   ├── library_loader.cpp
│   │   └── mod_loader.cpp
│   └── ui/               # UI layer implementation
│       └── main.cpp
├── resources/            # Assets and configuration
├── docs/                 # Documentation
└── build.sh             # Build script
```

## Migration Status

### ✅ Completed
- [x] Core engine structure integration
- [x] Native library loading with dlopen/dlsym
- [x] Platform abstraction layer
- [x] Mod loading system
- [x] CMake build system
- [x] Architecture detection and support

### 🔄 In Progress
- [ ] UI layer implementation
- [ ] Native windowing support (X11/Wayland)
- [ ] Graphics API shims
- [ ] Audio API abstractions
- [ ] Input system integration
- [ ] Version management and extraction
- [ ] Configuration system

## Next Steps

1. **UI Development**: Implement native Qt-based UI for game management
2. **Graphics Integration**: Add OpenGL/Vulkan rendering with Android shim
3. **Audio System**: Integrate ALSA/PulseAudio/PipeWire support
4. **Version Management**: Port version extraction and management logic
5. **Content Management**: Implement mod and resource management
6. **Testing**: Comprehensive testing across target architectures

## Technical Advantages

### Native Performance
- C++ implementation for maximum performance
- Direct system calls and API access
- No virtualization layer overhead

### Cross-Platform Compatibility
- Native Linux support with Android API shims
- x86_64 and aarch64 architecture support
- Wayland and X11 display server support

### Modularity
- Separate concerns for loading, core engine, and UI
- Pluggable architecture for easy extension
- Clean interfaces between components

### Performance Optimization
- Direct library loading with dlopen
- Thread-safe operations for concurrent access
- Minimal runtime overhead

## Usage

### Basic Usage
```bash
# Build the project
cd LeviLinux
./build.sh

# Run the launcher
./build/levilinux
```

### Development
```bash
# For debugging
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run with debug output
./levilinux --debug
```

## License
This project is licensed under the terms of the LGPL-3.0 License.
