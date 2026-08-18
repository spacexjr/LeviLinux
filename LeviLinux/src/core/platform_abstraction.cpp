#include "core/platform_abstraction.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <dlfcn.h>

PlatformAbstraction::PlatformAbstraction() {
    std::cout << "[Platform] PlatformAbstraction constructor" << std::endl;
    std::cout.flush();
}

PlatformAbstraction::~PlatformAbstraction() {
    std::cout << "[Platform] PlatformAbstraction destructor" << std::endl;
    std::cout.flush();
}

PlatformAbstraction& PlatformAbstraction::instance() {
    static PlatformAbstraction instance;
    return instance;
}

bool PlatformAbstraction::init() {
    if (initialized) {
        return true;
    }

    std::cout << "[Platform] Detecting platform..." << std::endl;
    detectPlatform();
    std::cout << "[Platform] Setting up graphics..." << std::endl;
    setupGraphicsShim();
    std::cout << "[Platform] Setting up audio..." << std::endl;
    setupAudioShim();
    std::cout << "[Platform] Setting up input..." << std::endl;
    setupInputShim();
    std::cout << "[Platform] Setting up Vulkan..." << std::endl;
    setupVulkanShim();
    std::cout << "[Platform] Setting up OpenGL ES..." << std::endl;
    setupOpenGLESShim();
    std::cout << "[Platform] Setting up ALSA..." << std::endl;
    setupALSAShim();
    std::cout << "[Platform] Setting up PulseAudio..." << std::endl;
    setupPulseAudioShim();
    std::cout << "[Platform] Setting up PipeWire..." << std::endl;
    setupPipeWireShim();

    initialized = true;
    std::cout << "[Platform] Initialization complete" << std::endl;
    return true;
}

void PlatformAbstraction::cleanup() {
    initialized = false;
}

std::string PlatformAbstraction::getGraphicsAPI() const {
    return detectedGraphicsAPI;
}

std::string PlatformAbstraction::getAudioAPI() const {
    return detectedAudioAPI;
}

std::string PlatformAbstraction::getInputAPI() const {
    return detectedInputAPI;
}

void PlatformAbstraction::detectPlatform() const {
#ifdef __linux__
    detectedGraphicsAPI = "linux";
    detectedAudioAPI = "linux";
    detectedInputAPI = "linux";
#endif
}

void PlatformAbstraction::setupGraphicsShim() const {
#ifdef __linux__
    const char* libGL[] = {"libGL.so.1", "libGL.so"};
    const char* libGLESv2[] = {"libGLESv2.so.2", "libGLESv2.so"};
    const char* libEGL[] = {"libEGL.so.1", "libEGL.so"};
    
    for (const char* lib : libGL) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            detectedGraphicsAPI = "opengl";
            break;
        }
    }
    
    if (detectedGraphicsAPI == "linux") {
        for (const char* lib : libGLESv2) {
            if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
                detectedGraphicsAPI = "opengles2";
                break;
            }
        }
    }
    
    if (detectedGraphicsAPI == "linux") {
        for (const char* lib : libEGL) {
            if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
                detectedGraphicsAPI = "egl";
                break;
            }
        }
    }
#endif
}

void PlatformAbstraction::setupAudioShim() const {
#ifdef __linux__
    const char* libPulse[] = {"libpulse.so.0", "libpulse.so"};
    const char* libALSA[] = {"libasound.so.2", "libasound.so"};
    const char* libPipeWire[] = {"libpipewire-0.3.so.0", "libpipewire-0.3.so"};
    
    for (const char* lib : libPulse) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            detectedAudioAPI = "pulseaudio";
            break;
        }
    }
    
    if (detectedAudioAPI == "linux") {
        for (const char* lib : libALSA) {
            if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
                detectedAudioAPI = "alsa";
                break;
            }
        }
    }
    
    if (detectedAudioAPI == "linux") {
        for (const char* lib : libPipeWire) {
            if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
                detectedAudioAPI = "pipewire";
                break;
            }
        }
    }
#endif
}

void PlatformAbstraction::setupInputShim() const {
#ifdef __linux__
    detectedInputAPI = "linux";
#endif
}

void PlatformAbstraction::setupVulkanShim() const {
#ifdef __linux__
    const char* libVulkan[] = {"libvulkan.so.1", "libvulkan.so"};
    for (const char* lib : libVulkan) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            break;
        }
    }
#endif
}

void PlatformAbstraction::setupOpenGLESShim() const {
#ifdef __linux__
    const char* libGLES[] = {"libGLESv1CM.so.1", "libGLESv1CM.so"};
    for (const char* lib : libGLES) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            break;
        }
    }
#endif
}

void PlatformAbstraction::setupALSAShim() const {
#ifdef __linux__
    const char* libAlsa[] = {"libasound.so.2", "libasound.so"};
    for (const char* lib : libAlsa) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            break;
        }
    }
#endif
}

void PlatformAbstraction::setupPulseAudioShim() const {
#ifdef __linux__
    const char* libPulse[] = {"libpulse.so.0", "libpulse.so"};
    for (const char* lib : libPulse) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            break;
        }
    }
#endif
}

void PlatformAbstraction::setupPipeWireShim() const {
#ifdef __linux__
    const char* libPipeWire[] = {"libpipewire-0.3.so.0", "libpipewire-0.3.so"};
    for (const char* lib : libPipeWire) {
        if (dlopen(lib, RTLD_LAZY | RTLD_LOCAL) != nullptr) {
            break;
        }
    }
#endif
}