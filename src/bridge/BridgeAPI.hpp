/**
 * LeviLinux Bridge API - C++ Implementation
 * Connects the C bridge API to C++ service layer
 */
#pragma once

#include "../core/launcher/Services.hpp"
#include "types.h"
#include "callbacks.h"

#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace levi {

// ─────────────────────────────────────────────────────────────────────────────
// BRIDGE API IMPLEMENTATION
// ─────────────────────────────────────────────────────────────────────────────

class BridgeAPI {
public:
    static BridgeAPI& instance();

    /**
     * Initialize all services
     */
    bool initialize(LeviLogCallback log_cb, void* user_data);

    /**
     * Shutdown all services
     */
    void shutdown();

    /**
     * Check if initialized
     */
    bool isInitialized() const { return initialized_.load(); }

    // ── Version Operations ─────────────────────────────────────────────────

    const LeviVersionList* versionsRefresh(LeviProgressCallback progress_cb, void* user_data);
    const LeviVersionInfo* versionsGetInfo(const char* version_id);
    bool versionsIsValid(const char* version_id);
    LeviErrorCode versionsDelete(const char* version_id);
    LeviErrorCode versionsExtract(const char* apk_path, const char* version_id,
                                   LeviProgressCallback progress_cb, void* user_data);
    LeviErrorCode versionsSetArgs(const char* version_id, const char* args);

    // ── Mod Operations ─────────────────────────────────────────────────────

    LeviErrorCode modsSetVersion(const char* version_id);
    const LeviModList* modsGetList(void);
    const LeviModInfo* modsGetInfo(const char* mod_id);
    LeviErrorCode modsSetEnabled(const char* mod_id, bool enabled);
    LeviErrorCode modsReorder(const char** mod_ids, size_t count);
    LeviErrorCode modsDelete(const char* mod_id);
    LeviErrorCode modsInstall(const char* mod_path,
                               LeviProgressCallback progress_cb, void* user_data);
    void modsSetEventCallback(LeviModEventCallback callback, void* user_data);

    // ── Auth Operations ────────────────────────────────────────────────────

    LeviErrorCode authLogin(LeviAuthCallback auth_cb, void* user_data);
    LeviErrorCode authLogout(void);
    const LeviAccountInfo* authGetAccount(void);
    const LeviAuthTokens* authGetTokens(void);
    bool authIsLoggedIn(void);

    // ── Game Launch ────────────────────────────────────────────────────────

    LeviErrorCode launchGame(const LeviLaunchConfig* config,
                              LeviGameStateCallback game_state_cb, void* user_data);
    bool gameIsRunning(void);
    int gameGetPid(void);
    LeviErrorCode gameTerminate(void);
    void gameSetLogCallback(LeviLogCallback callback, void* user_data);

    // ── Settings ───────────────────────────────────────────────────────────

    LeviErrorCode settingsLoad(void);
    LeviErrorCode settingsSave(void);
    const LeviSettings* settingsGet(void);
    LeviErrorCode settingsUpdate(const LeviSettings* settings);

    // ── Utilities ──────────────────────────────────────────────────────────

    LeviErrorCode openDirectory(const char* path);
    LeviErrorCode openUrl(const char* url);
    LeviErrorCode cleanupCache(void);

private:
    BridgeAPI() = default;
    ~BridgeAPI() = default;
    BridgeAPI(const BridgeAPI&) = delete;
    BridgeAPI& operator=(const BridgeAPI&) = delete;

    struct Impl {
        // Placeholder for future service integrations
    };
    std::unique_ptr<Impl> pimpl_;

    std::atomic<bool> initialized_{false};

    // Callback storage (C function pointers + user data)
    struct CallbackEntry {
        void* user_data = nullptr;
    };

    struct LogCallbackEntry : CallbackEntry {
        LeviLogCallback callback = nullptr;
    };

    struct GameStateCallbackEntry : CallbackEntry {
        LeviGameStateCallback callback = nullptr;
    };

    struct ModEventCallbackEntry : CallbackEntry {
        LeviModEventCallback callback = nullptr;
    };

    LogCallbackEntry log_cb_;
    GameStateCallbackEntry game_state_cb_;
    ModEventCallbackEntry mod_event_cb_;

    // Internal data storage
    mutable std::mutex data_mutex_;
    std::unordered_map<std::string, LeviVersionInfo> versions_cache_;
    std::unordered_map<std::string, LeviModInfo> mods_cache_;
    LeviAccountInfo account_cache_{};
    LeviAuthTokens tokens_cache_{};
    LeviSettings settings_cache_{};

    // Convert between internal types and bridge types
    void syncVersionsFromService();
    void syncModsFromService();
    void syncAuthFromService();
};

} // namespace levi