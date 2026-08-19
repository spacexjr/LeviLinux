/**
 * LeviLinux Mod Manifest Parser
 * Parses manifest.json (port from LeviLaunchroid)
 */
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace levi::mod {

struct ModManifest {
    static constexpr const char* FILENAME = "manifest.json";
    static constexpr const char* TYPE_PRELOAD_NATIVE = "preload-native";
    
    std::string type;               // "preload-native"
    std::string name;               // Display name
    std::string entry;              // Entry .so file (e.g., "libmod.so")
    std::string author = "Unknown";
    std::string icon = "";
    std::string version = "1.0.0";
    std::string description = "";
    std::vector<std::string> minecraft_versions;  // Version patterns
    
    // Parsed from JSON
    nlohmann::json raw_json;
    
    // Validation
    bool isValid() const;
    bool isPreloadNative() const { return type == TYPE_PRELOAD_NATIVE; }
    
    // Version compatibility (supports wildcards like "1.21.*")
    bool isCompatible(const std::string& mc_version) const;
    
    // Serialization
    nlohmann::json toJson() const;
    static std::optional<ModManifest> fromJson(const nlohmann::json& json);
    static std::optional<ModManifest> fromFile(const std::filesystem::path& path);
    static std::optional<ModManifest> fromString(const std::string& json_str);
    
    // Fingerprinting for cache invalidation
    std::string computeFingerprint(const std::filesystem::path& mod_root) const;
};

// ─────────────────────────────────────────────────────────────────────────────
// CACHE MANIFEST (for incremental loading)
// ─────────────────────────────────────────────────────────────────────────────

struct CacheManifest {
    static constexpr const char* FILENAME = ".mod_cache_manifest";
    static constexpr int VERSION = 4;
    
    int loader_version = VERSION;
    std::string mod_id;
    std::string entry_path;
    std::string fingerprint;        // Hash of all mod files
    
    struct FileEntry {
        std::string path;           // Relative path
        uint64_t size = 0;
        uint64_t mtime = 0;         // Modification time
        std::string hash;           // SHA256 (optional)
    };
    std::vector<FileEntry> files;
    
    bool isValid() const;
    bool matches(const CacheManifest& other) const;
    
    std::string serialize() const;
    static std::optional<CacheManifest> deserialize(const std::string& data);
    static std::optional<CacheManifest> fromFile(const std::filesystem::path& path);
    bool toFile(const std::filesystem::path& path) const;
};

} // namespace levi::mod