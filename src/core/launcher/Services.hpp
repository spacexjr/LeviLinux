/**
 * LeviLinux Launch Service
 * High-level launch orchestration used by UI and Bridge API
 * Coordinates mods, auth, and game launcher
 */
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <cstdint>

namespace levi {

class GameLauncher;
class ModLoader;
class ModManager;

// ─────────────────────────────────────────────────────────────────────────────
// LAUNCH SERVICE
// ─────────────────────────────────────────────────────────────────────────────

class LaunchService {
public:
    struct LaunchRequest {
        std::string version_id;
        std::string auth_xuid;
        std::string auth_access_token;
        std::string auth_identity_token;
        bool disable_mods = false;
        std::string extra_args;
        std::string extra_env;
        int gpu_choice = 0;
    };

    struct LaunchResult {
        bool success = false;
        int pid = 0;
        std::string error_message;
    };

    enum class Stage {
        Idle = 0,
        Validating,
        LoadingMods,
        InitializingPreloader,
        Authenticating,
        StartingProcess,
        WaitingWindow,
        Running,
        Failed,
    };

    using LogCallback = std::function<void(int level, const std::string& message)>;
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using StateCallback = std::function<void(Stage stage, int exit_code)>;

    LaunchService();
    ~LaunchService();

    LaunchService(const LaunchService&) = delete;
    LaunchService& operator=(const LaunchService&) = delete;

    /**
     * Launch the game with the given request
     * This is the main entry point called from BridgeAPI or UI
     */
    LaunchResult launch(const LaunchRequest& request,
                        LogCallback log_cb = nullptr,
                        ProgressCallback progress_cb = nullptr,
                        StateCallback state_cb = nullptr);

    /**
     * Check if game is currently running
     */
    bool isRunning() const { return running_.load(); }

    /**
     * Get current stage
     */
    Stage getStage() const { return stage_.load(); }

    /**
     * Terminate the game
     */
    bool terminate();

    /**
     * Wait for game to exit
     */
    int waitForExit(int timeout_ms = -1);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;

    std::atomic<bool> running_{false};
    std::atomic<Stage> stage_{Stage::Idle};

    std::unique_ptr<GameLauncher> launcher_;
    std::unique_ptr<ModLoader> mod_loader_;
    std::unique_ptr<ModManager> mod_manager_;
};

// ─────────────────────────────────────────────────────────────────────────────
// VERSION SERVICE
// ─────────────────────────────────────────────────────────────────────────────

class VersionService {
public:
    struct VersionInfo {
        std::string id;
        std::string name;
        std::string path;
        std::string architecture;
        bool valid = false;
        bool installed = false;
        int64_t size_bytes = 0;
        int64_t last_played = 0;
    };

    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using LogCallback = std::function<void(int level, const std::string& message)>;

    VersionService();
    ~VersionService();

    VersionService(const VersionService&) = delete;
    VersionService& operator=(const VersionService&) = delete;

    /**
     * Refresh list of installed versions
     */
    void refresh();

    /**
     * Get all installed versions
     */
    std::vector<VersionInfo> getVersions() const;

    /**
     * Get info for specific version
     */
    VersionInfo getVersionInfo(const std::string& version_id) const;

    /**
     * Check if version is valid (has libminecraftpe.so)
     */
    bool isVersionValid(const std::string& version_id) const;

    /**
     * Extract version from APK/TMC file
     */
    bool extractVersion(const std::string& apk_path, const std::string& version_id,
                        std::string& error_out, ProgressCallback progress_cb = nullptr);

    /**
     * Delete version
     */
    bool deleteVersion(const std::string& version_id, std::string& error_out);

    /**
     * Get default data root
     */
    static std::string getDataRoot();

    /**
     * Get host architecture string
     */
    static std::string getHostArchitecture();

    /**
     * Check if APK is compatible with host architecture
     */
    static bool isApkCompatible(const std::string& apk_path, std::string& error_out);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// ─────────────────────────────────────────────────────────────────────────────
// MOD SERVICE
// ─────────────────────────────────────────────────────────────────────────────

class ModService {
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

    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using LogCallback = std::function<void(int level, const std::string& message)>;
    using ModEventCallback = std::function<void(const std::string& mod_id, int event_type)>;

    ModService();
    ~ModService();

    ModService(const ModService&) = delete;
    ModService& operator=(const ModService&) = delete;

    /**
     * Set current version for mod management
     */
    void setVersion(const std::string& version_id);

    /**
     * Refresh mod list
     */
    void refresh();

    /**
     * Get all mods for current version
     */
    std::vector<ModInfo> getMods() const;

    /**
     * Enable/disable a mod
     */
    bool setModEnabled(const std::string& mod_id, bool enabled);

    /**
     * Reorder mods
     */
    bool reorderMods(const std::vector<std::string>& mod_ids);

    /**
     * Delete a mod
     */
    bool deleteMod(const std::string& mod_id);

    /**
     * Install mod from path
     */
    bool installMod(const std::string& source_path,
                    std::string& error_out,
                    ProgressCallback progress_cb = nullptr);

    /**
     * Get mod info by ID
     */
    ModInfo getModInfo(const std::string& mod_id) const;

    /**
     * Set mod event callback
     */
    void setModEventCallback(ModEventCallback callback) { event_cb_ = std::move(callback); }

    /**
     * Get default mods directory for a version
     */
    static std::string getModsDir(const std::string& version_id);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    std::string current_version_;
    ModEventCallback event_cb_;
};

// ─────────────────────────────────────────────────────────────────────────────
// AUTH SERVICE
// ─────────────────────────────────────────────────────────────────────────────

class AuthService {
public:
    struct AuthTokens {
        std::string xuid;
        std::string gamertag;
        std::string access_token;
        std::string identity_token;
        int64_t expires_at = 0;
        std::string display_name;
        std::string avatar_url;
    };

    struct AccountInfo {
        std::string email;
        std::string xuid;
        std::string gamertag;
        std::string avatar_url;
        bool is_valid = false;
    };

    using LogCallback = std::function<void(int level, const std::string& message)>;
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using AuthCallback = std::function<void(bool success, const std::string& xuid,
                                            const std::string& gamertag)>;

    AuthService();
    ~AuthService();

    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;

    /**
     * Start MSA login flow
     */
    bool login(AuthCallback auth_cb = nullptr, ProgressCallback progress_cb = nullptr);

    /**
     * Logout current account
     */
    bool logout();

    /**
     * Get current account
     */
    AccountInfo getAccount() const;

    /**
     * Get valid tokens (refresh if needed)
     */
    AuthTokens getTokens();

    /**
     * Check if logged in
     */
    bool isLoggedIn() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// ─────────────────────────────────────────────────────────────────────────────
// SETTINGS SERVICE
// ─────────────────────────────────────────────────────────────────────────────

class SettingsService {
public:
    struct Settings {
        std::string versions_dir;
        std::string mods_dir;
        std::string cache_dir;
        std::string logs_dir;
        bool auto_update = true;
        bool minimize_on_launch = true;
        bool close_on_game_exit = false;
        int log_level = 1;
        std::string language = "en";
        // Renderer
        bool force_vibrants = false;
        bool old_intel = false;
        bool nvidia_optimus = false;
        bool zink = false;
        bool glvk_fps = false;
        bool black_screen_fix = false;
        // GPU
        std::string gpu_choice = "auto";  // "auto", "igpu", "dgpu"
    };

    SettingsService();
    ~SettingsService();

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    /**
     * Load settings from disk
     */
    bool load();

    /**
     * Save settings to disk
     */
    bool save();

    /**
     * Get current settings
     */
    const Settings& get() const { return settings_; }

    /**
     * Update settings (partial update)
     */
    void update(const Settings& settings);

    /**
     * Reset to defaults
     */
    void resetToDefaults();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    Settings settings_;
};

} // namespace levi