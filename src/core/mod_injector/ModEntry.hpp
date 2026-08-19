/**
 * LeviLinux Mod Entry
 * Represents a mod ready for injection into the game process
 */
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

namespace levi::mod {

struct ModEntry {
    std::string id;                 // Mod ID (directory name)
    std::string name;               // Display name from manifest
    std::string version;            // Mod version
    std::string author;             // Mod author
    std::string description;        // Mod description
    
    // Paths
    std::string lib_path;           // Full path to mod .so (e.g., /cache/mods/id/libmod.so)
    std::string root_path;          // Mod root directory (e.g., /cache/mods/id/)
    std::string config_path;        // Config directory (e.g., /cache/mods/id/config/)
    std::string data_path;          // Data directory (e.g., /cache/mods/id/data/)
    std::string icon_path;          // Optional icon
    
    // Manifest info
    std::string entry_point;        // Entry file from manifest (e.g., "libmod.so")
    std::vector<std::string> minecraft_versions;  // Supported MC versions
    std::string mod_type;           // "preload-native", "script", etc.
    
    // Runtime state
    bool enabled = true;
    bool loaded = false;
    int load_order = 0;             // 0 = first
    void* handle = nullptr;         // dlopen handle
    
    // Config schema (from pl::config)
    std::string config_schema_json; // Serialized schema for UI
    
    // Constructor
    ModEntry() = default;
    ModEntry(const ModEntry&) = default;
    ModEntry& operator=(const ModEntry&) = default;
    ModEntry(ModEntry&&) noexcept = default;
    ModEntry& operator=(ModEntry&&) noexcept = default;
    
    // Validation
    bool isValid() const;
    bool isCompatible(const std::string& mc_version) const;
    
    // Config file helpers
    std::string getConfigFile(const std::string& filename) const;
    std::vector<std::string> listConfigFiles() const;
    std::vector<std::string> listDataFiles() const;
};

// ─────────────────────────────────────────────────────────────────────────────
// MOD REGISTRY (Runtime registry of loaded mods in game process)
// ─────────────────────────────────────────────────────────────────────────────

class ModRegistry {
public:
    struct RegisteredMod {
        std::string id;
        std::string name;
        std::string version;
        void* handle = nullptr;           // dlopen handle
        void* mod_instance = nullptr;     // pl::mod::NativeMod*
        bool enabled = false;
    };
    
    static ModRegistry& instance();
    
    // Called from Preloader during JNI_OnLoad
    bool registerMod(const ModEntry& entry, void* handle, void* mod_instance);
    bool unregisterMod(const std::string& mod_id);
    
    // Mod lifecycle (called from Preloader)
    bool enableMod(const std::string& mod_id);
    bool disableMod(const std::string& mod_id);
    
    // Queries
    const RegisteredMod* getMod(const std::string& mod_id) const;
    std::vector<RegisteredMod> getAllMods() const;
    std::vector<RegisteredMod> getEnabledMods() const;
    
    // Symbol lookup across all loaded mods
    void* findSymbol(const std::string& symbol) const;
    
    // Clear all (on game shutdown)
    void clear();

private:
    ModRegistry() = default;
    ~ModRegistry() = default;
    ModRegistry(const ModRegistry&) = delete;
    ModRegistry& operator=(const ModRegistry&) = delete;
    
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, RegisteredMod> mods_;
};

} // namespace levi::mod