#include "core/core_engine.hpp"
#include "loader/library_loader.hpp"
#include "loader/mod_loader.hpp"
#include "core/platform_abstraction.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>

struct CoreEngine::Impl {
    LibraryLoader libraryLoader;
    ModLoader modLoader;
    PlatformAbstraction platformAbstraction;
    std::string platformInfo;
    
    std::vector<std::string> coreLibraryPaths;
};

CoreEngine::CoreEngine() : pImpl(std::make_unique<Impl>()) {
    std::cout << "[Core] CoreEngine constructor - creating Impl..." << std::endl;
    std::cout.flush();
    pImpl->coreLibraryPaths = {
        "minecraftpe",
        "mcpelauncher-client",
        "mcpelauncher-client86",
        "libminecraftpe.so",
        "libminecraftpe.so.1",
        "libminecraftpe.so.2"
    };
    std::cout << "[Core] CoreEngine constructor done" << std::endl;
    std::cout.flush();
}

CoreEngine::~CoreEngine() {}

bool CoreEngine::initialize() {
    std::cout << "[Core] Initializing platform abstraction..." << std::endl;
    std::cout.flush();
    if (!pImpl->platformAbstraction.init()) {
        std::cerr << "Failed to initialize platform abstraction" << std::endl;
        return false;
    }
    std::cout << "[Core] Platform abstraction initialized" << std::endl;
    std::cout.flush();

    std::string graphicsAPI = pImpl->platformAbstraction.getGraphicsAPI();
    std::string audioAPI = pImpl->platformAbstraction.getAudioAPI();
    std::string inputAPI = pImpl->platformAbstraction.getInputAPI();
    int archBits = sizeof(void*) * 8;

    pImpl->platformInfo = std::string("Platform: Linux\n") +
                         "Graphics API: " + graphicsAPI + "\n" +
                         "Audio API: " + audioAPI + "\n" +
                         "Input API: " + inputAPI + "\n" +
                         "Architecture: " + std::to_string(archBits) + "-bit\n" +
                         "Compiler: " __VERSION__;

    return true;
}

void CoreEngine::cleanup() {
    pImpl->platformAbstraction.cleanup();
    pImpl->modLoader.disableAndUnloadMods();
}

std::string CoreEngine::getPlatformInfo() const {
    return pImpl->platformInfo;
}

bool CoreEngine::loadCoreLibraries(const std::string& path) {
    bool success = false;
    for (const auto& libName : pImpl->coreLibraryPaths) {
        if (pImpl->libraryLoader.loadLibrary(libName, path)) {
            success = true;
        }
    }
    return success;
}

bool CoreEngine::loadMods(const std::vector<std::string>& modPaths) {
    bool result = true;
    for (const auto& modPath : modPaths) {
        if (!pImpl->modLoader.loadMod(modPath)) {
            std::cerr << "Failed to load mod: " << modPath << std::endl;
            result = false;
        }
    }
    return result;
}

void CoreEngine::enableAllMods() {
    pImpl->modLoader.enableMods();
}

void CoreEngine::disableAndUnloadAllMods() {
    pImpl->modLoader.disableAndUnloadMods();
}

LibraryLoader& CoreEngine::getLibraryLoader() {
    return pImpl->libraryLoader;
}

ModLoader& CoreEngine::getModLoader() {
    return pImpl->modLoader;
}

PlatformAbstraction& CoreEngine::getPlatformAbstraction() {
    return pImpl->platformAbstraction;
}