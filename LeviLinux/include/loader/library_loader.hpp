#ifndef LIBRARY_LOADER_HPP
#define LIBRARY_LOADER_HPP

#include <string>
#include <unordered_map>
#include <memory>

class LibraryLoader {
public:
    LibraryLoader();
    ~LibraryLoader();
    LibraryLoader(const LibraryLoader&) = delete;
    LibraryLoader& operator=(const LibraryLoader&) = delete;

    bool loadLibrary(const std::string& name, const std::string& path = "");
    void* getSymbol(const std::string& name) const;
    bool isLoaded(const std::string& name) const;
    void unloadLibrary(const std::string& name);

    static LibraryLoader& instance();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // LIBRARY_LOADER_HPP
