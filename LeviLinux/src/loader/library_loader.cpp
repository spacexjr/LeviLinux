#include "loader/library_loader.hpp"
#include <dlfcn.h>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <unordered_map>

struct LibraryLoader::Impl {
    std::unordered_map<std::string, void*> handles;
    std::mutex mutex;
};

LibraryLoader::LibraryLoader() : pImpl(std::make_unique<Impl>()) {
    std::cout << "[Loader] LibraryLoader constructor" << std::endl;
    std::cout.flush();
}
LibraryLoader::~LibraryLoader() {
    for (auto& [name, handle] : pImpl->handles) {
        if (handle) {
            dlclose(handle);
        }
    }
}

bool LibraryLoader::loadLibrary(const std::string& name, const std::string& path) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    if (isLoaded(name)) {
        return true;
    }

    std::string libPath = path.empty() ? "lib" + name + ".so" : path;
    void* handle = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if (!handle) {
        std::cerr << "Failed to load library " << libPath << ": " << dlerror() << std::endl;
        return false;
    }

    pImpl->handles[name] = handle;
    return true;
}

void* LibraryLoader::getSymbol(const std::string& name) const {
    void* handle = nullptr;

    {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        handle = pImpl->handles.at(name);
    }

    if (!handle) {
        return nullptr;
    }

    return dlsym(handle, name.c_str());
}

bool LibraryLoader::isLoaded(const std::string& name) const {
    std::lock_guard<std::mutex> lock(pImpl->mutex);
    return pImpl->handles.find(name) != pImpl->handles.end() && pImpl->handles.at(name) != nullptr;
}

void LibraryLoader::unloadLibrary(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl->mutex);

    auto it = pImpl->handles.find(name);
    if (it != pImpl->handles.end() && it->second) {
        dlclose(it->second);
        it->second = nullptr;
    }
}

LibraryLoader& LibraryLoader::instance() {
    static LibraryLoader instance;
    return instance;
}
