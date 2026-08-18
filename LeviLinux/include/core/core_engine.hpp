#ifndef CORE_ENGINE_HPP
#define CORE_ENGINE_HPP

#include <memory>
#include <string>
#include <vector>

class LibraryLoader;
class ModLoader;
class PlatformAbstraction;

class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine();

    bool initialize();
    void cleanup();

    std::string getPlatformInfo() const;
    bool loadCoreLibraries(const std::string& path = "");
    bool loadMods(const std::vector<std::string>& modPaths);
    void enableAllMods();
    void disableAndUnloadAllMods();

    LibraryLoader& getLibraryLoader();
    ModLoader& getModLoader();
    PlatformAbstraction& getPlatformAbstraction();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // CORE_ENGINE_HPP
