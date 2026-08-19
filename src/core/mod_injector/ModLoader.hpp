/**
 * LeviLinux Mod Loader
 * Port of LeviLaunchroid's ModNativeLoader + ModManager to C++
 * Integrates with Trinity's mcpelauncher launch pipeline
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include "ModEntry.hpp"
#include "ModManifest.hpp"
#include "Preloader.hpp"

namespace levi::mod {

using ProgressCallback = std::function<void(float progress, const std::string& status)>;
using LogCallback = std::function<void(int level, const std::string& message)>;

class ModLoader {
public:
    struct LoadResult {
        bool success = false;
        std::string error_message;
        std::vector<ModEntry> loaded_mods;
    };

    struct LoadOptions {
        std::string version_id;
        std::string mods_dir;           // ~/.local/share/LeviLinux/mods/<version>/
        std::string cache_dir;          // ~/.cache/LeviLinux/mods/
        std::vector<std::string> enabled_mod_ids;  // Ordered list
        bool validate_compatibility = true;
        bool use_cache = true;
    };

    ModLoader();
    ~ModLoader();

    // Non-copyable, movable
    ModLoader(const ModLoader&) = delete;
    ModLoader& operator=(const ModLoader&) = delete;
    ModLoader(ModLoader&&) noexcept = default;
    ModLoader& operator=(ModLoader&&) noexcept = default;

    /**
     * Load all enabled mods for a version
     * This is called by LaunchService before starting the game
     */
    LoadResult loadMods(const LoadOptions& options,
                        ProgressCallback progress_cb = nullptr,
                        LogCallback log_cb = nullptr);

    /**
     * Unload all mods (called on game exit)
     */
    void unloadAllMods();

    /**
     * Get currently loaded mods
     */
    const std::vector<ModEntry>& getLoadedMods() const { return loaded_mods_; }

    /**
     * Check if a mod is loaded
     */
    bool isModLoaded(const std::string& mod_id) const;

    /**
     * Get mod entry by ID
     */
    const ModEntry* getModEntry(const std::string& mod_id) const;

    /**
     * Set preloader instance (for JNI_OnLoad hook)
     */
    void setPreloader(Preloader* preloader) { preloader_ = preloader; }

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    std::vector<ModEntry> loaded_mods_;
    Preloader* preloader_ = nullptr;
    mutable std::shared_mutex mods_mutex_;
};

// ─────────────────────────────────────────────────────────────────────────────
// MOD MANAGER (Higher-level management: enable/disable, ordering, config)
// ─────────────────────────────────────────────────────────────────────────────

class ModManager {
public:
    struct ModInfo {
        std::string id;
        std::string name;
        std::string version;
        std::string author;
        std::string description;
        std::string entry_path;
        std::string root_path;
        std::string config_path;
        std::string icon_path;
        std::vector<std::string> minecraft_versions;
        bool enabled = true;
        bool compatible = true;
        bool has_config = false;
        int config_file_count = 0;
        int load_order = 0;
    };

    using ModList = std::vector<ModInfo>;
    using ModEventCallback = std::function<void(const std::string& mod_id, int event_type)>;

    ModManager();
    ~ModManager();

    ModManager(const ModManager&) = delete;
    ModManager& operator=(const ModManager&) = delete;

    /**
     * Set the current Minecraft version for mod management
     * Scans the version's mods directory
     */
    bool setCurrentVersion(const std::string& version_id, const std::string& base_mods_dir);

    /**
     * Get current version ID
     */
    const std::string& getCurrentVersion() const { return current_version_; }

    /**
     * Refresh mod list from disk
     */
    void refresh();

    /**
     * Get all discovered mods
     */
    const ModList& getMods() const { return mods_; }

    /**
     * Enable/disable a mod
     */
    bool setModEnabled(const std::string& mod_id, bool enabled);

    /**
     * Reorder mods (changes load order)
     */
    bool reorderMods(const std::vector<std::string>& mod_ids);

    /**
     * Delete a mod
     */
    bool deleteMod(const std::string& mod_id);

    /**
     * Install mod from path (directory or zip)
     */
    bool installMod(const std::string& source_path, ProgressCallback progress_cb = nullptr);

    /**
     * Set mod event callback
     */
    void setModEventCallback(ModEventCallback callback) { event_callback_ = std::move(callback); }

    /**
     * Save mod configuration (enabled state, order)
     */
    bool saveConfig();

    /**
     * Load mod configuration
     */
    bool loadConfig();

    /**
     * Get mod info by ID
     */
    const ModInfo* getModInfo(const std::string& mod_id) const;

    /**
     * Check if mod is compatible with current MC version
     */
    static bool isCompatible(const ModInfo& mod, const std::string& mc_version);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    std::string current_version_;
    std::string base_mods_dir_;
    ModList mods_;
    ModEventCallback event_callback_;
    mutable std::shared_mutex mods_mutex_;
    
    void notifyEvent(const std::string& mod_id, int event_type);
    void invalidateCache();
};

} // namespace levi::mod