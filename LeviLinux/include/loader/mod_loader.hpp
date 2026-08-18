#ifndef MOD_LOADER_HPP
#define MOD_LOADER_HPP

#include <string>
#include <vector>
#include <memory>

class ModLoader {
public:
    ModLoader();
    ~ModLoader();

    bool loadMod(const std::string& modPath);
    bool enableMods();
    bool disableAndUnloadMods();
    bool isModLoaded(const std::string& modPath) const;
    std::vector<std::string> getLoadedMods() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // MOD_LOADER_HPP
