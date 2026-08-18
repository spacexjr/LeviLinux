#include "loader/mod_loader.hpp"
#include "loader/library_loader.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <dlfcn.h>
#include <iostream>

struct ModLoader::Impl {
    LibraryLoader& libraryLoader = LibraryLoader::instance();
    std::unordered_map<std::string, void*> modHandles;
    std::vector<std::string> loadedMods;
    std::mutex mutex;
    
    bool (*nativeLoadMod)(const char* libPath, const char* modRootPath, void* modObj) = nullptr;
    void (*nativeEnableLoadedMods)() = nullptr;
    void (*nativeDisableAndUnloadLoadedMods)() = nullptr;
};

ModLoader::ModLoader() : pImpl(std::make_unique<Impl>()) {
    std::cout << "[ModLoader] ModLoader constructor start" << std::endl;
    std::cout.flush();
    pImpl->libraryLoader.loadLibrary("preloader");
    std::cout << "[ModLoader] loadLibrary done" << std::endl;
    std::cout.flush();
    pImpl->nativeLoadMod = (bool (*)(const char*, const char*, void*))pImpl->libraryLoader.getSymbol("nativeLoadMod");
    pImpl->nativeEnableLoadedMods = (void (*)())pImpl->libraryLoader.getSymbol("nativeEnableLoadedMods");
    pImpl->nativeDisableAndUnloadLoadedMods = (void (*)())pImpl->libraryLoader.getSymbol("nativeDisableAndUnloadLoadedMods");
    std::cout << "[ModLoader] ModLoader constructor done" << std::endl;
    std::cout.flush();
}

ModLoader::~ModLoader() {}

bool ModLoader::loadMod(const std::string& modPath) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    
    if (isModLoaded(modPath)) {
        return true;
    }
    
    void* handle = nullptr;
    if (pImpl->nativeLoadMod) {
        void* modObj = nullptr;
        bool result = pImpl->nativeLoadMod(modPath.c_str(), "", modObj);
        if (result && modObj) {
            pImpl->modHandles[modPath] = modObj;
            pImpl->loadedMods.push_back(modPath);
            return true;
        }
    }
    
    return false;
}

bool ModLoader::enableMods() {
    if (pImpl->nativeEnableLoadedMods) {
        pImpl->nativeEnableLoadedMods();
        return true;
    }
    return false;
}

bool ModLoader::disableAndUnloadMods() {
    if (pImpl->nativeDisableAndUnloadLoadedMods) {
        pImpl->nativeDisableAndUnloadLoadedMods();
        return true;
    }
    return false;
}

bool ModLoader::isModLoaded(const std::string& modPath) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->modHandles.find(modPath) != pImpl->modHandles.end();
}

std::vector<std::string> ModLoader::getLoadedMods() const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->loadedMods;
}
