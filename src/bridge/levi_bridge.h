/**
 * LeviLinux Bridge API - Main C API
 * This is the public interface for UI ↔ Core communication
 */
#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ─────────────────────────────────────────────────────────────────────────────
// LIFECYCLE
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Initialize the LeviLinux core library
 * Must be called before any other API function
 * @param log_callback Callback for log messages (can be NULL)
 * @param user_data User data passed to callbacks
 * @return LEVI_OK on success, error code on failure
 */
LeviErrorCode levi_init(LeviLogCallback log_callback, void* user_data);

/**
 * Shutdown the LeviLinux core library
 * Releases all resources, stops background threads
 */
void levi_shutdown(void);

/**
 * Get library version info
 */
const LeviLinuxVersion* levi_get_version(void);

/**
 * Get last error message (thread-local)
 */
const char* levi_get_last_error(void);

// ─────────────────────────────────────────────────────────────────────────────
// SETTINGS
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Load settings from config file
 */
LeviErrorCode levi_settings_load(void);

/**
 * Save settings to config file
 */
LeviErrorCode levi_settings_save(void);

/**
 * Get current settings (returns pointer to internal struct, do not free)
 */
const LeviSettings* levi_settings_get(void);

/**
 * Update settings (partial update, only non-NULL/non-empty fields)
 */
LeviErrorCode levi_settings_update(const LeviSettings* settings);

// ─────────────────────────────────────────────────────────────────────────────
// VERSION MANAGEMENT
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Refresh version list from disk
 * @param progress_cb Optional progress callback
 * @param user_data User data for callback
 */
LeviErrorCode levi_versions_refresh(LeviProgressCallback progress_cb, void* user_data);

/**
 * Get list of installed versions
 * @return Pointer to internal list, valid until next refresh
 */
const LeviVersionList* levi_versions_get_list(void);

/**
 * Get info for specific version
 */
const LeviVersionInfo* levi_versions_get_info(const char* version_id);

/**
 * Check if version is valid (has libminecraftpe.so)
 */
bool levi_versions_is_valid(const char* version_id);

/**
 * Delete a version installation
 */
LeviErrorCode levi_versions_delete(const char* version_id);

/**
 * Extract/install a version from APK/TMC file
 * @param apk_path Path to .apk or .tmc file
 * @param version_id Target version ID (directory name)
 * @param progress_cb Progress callback
 * @param user_data User data for callback
 */
LeviErrorCode levi_versions_extract(const char* apk_path, const char* version_id,
                                     LeviProgressCallback progress_cb, void* user_data);

/**
 * Edit version launch arguments
 */
LeviErrorCode levi_versions_set_args(const char* version_id, const char* args);

// ─────────────────────────────────────────────────────────────────────────────
// MOD MANAGEMENT
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Set current version for mod management
 * Must be called before mod operations
 */
LeviErrorCode levi_mods_set_version(const char* version_id);

/**
 * Refresh mod list for current version
 */
LeviErrorCode levi_mods_refresh(void);

/**
 * Get list of mods for current version
 */
const LeviModList* levi_mods_get_list(void);

/**
 * Get info for specific mod
 */
const LeviModInfo* levi_mods_get_info(const char* mod_id);

/**
 * Enable/disable a mod
 */
LeviErrorCode levi_mods_set_enabled(const char* mod_id, bool enabled);

/**
 * Reorder mods (load order)
 * @param mod_ids Array of mod IDs in new order
 * @param count Number of mod IDs
 */
LeviErrorCode levi_mods_reorder(const char** mod_ids, size_t count);

/**
 * Delete a mod
 */
LeviErrorCode levi_mods_delete(const char* mod_id);

/**
 * Install a mod from directory or archive
 * @param mod_path Path to mod directory or .zip
 * @param progress_cb Progress callback
 * @param user_data User data for callback
 */
LeviErrorCode levi_mods_install(const char* mod_path,
                                 LeviProgressCallback progress_cb, void* user_data);

/**
 * Set mod event callback
 */
void levi_mods_set_event_callback(LeviModEventCallback callback, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// AUTHENTICATION (MSA / Xbox Live)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Start MSA authentication flow
 * Opens browser/device code flow, calls auth_cb when done
 */
LeviErrorCode levi_auth_login(LeviAuthCallback auth_cb, void* user_data);

/**
 * Logout current account
 */
LeviErrorCode levi_auth_logout(void);

/**
 * Get current account info
 */
const LeviAccountInfo* levi_auth_get_account(void);

/**
 * Get valid tokens (refreshes if needed)
 * @return Pointer to internal tokens, valid until logout
 */
const LeviAuthTokens* levi_auth_get_tokens(void);

/**
 * Check if authenticated
 */
bool levi_auth_is_logged_in(void);

// ─────────────────────────────────────────────────────────────────────────────
// GAME LAUNCH
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Launch Minecraft Bedrock with given configuration
 * @param config Launch configuration
 * @param game_state_cb Callback for game state changes
 * @param user_data User data for callback
 * @return LEVI_OK if launch started, error code on failure
 */
LeviErrorCode levi_launch_game(const LeviLaunchConfig* config,
                                LeviGameStateCallback game_state_cb, void* user_data);

/**
 * Check if game is currently running
 */
bool levi_game_is_running(void);

/**
 * Get PID of running game process (0 if not running)
 */
int levi_game_get_pid(void);

/**
 * Force terminate game process
 */
LeviErrorCode levi_game_terminate(void);

/**
 * Set log callback for game output
 */
void levi_game_set_log_callback(LeviLogCallback callback, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// UTILITIES
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Open directory in file manager
 */
LeviErrorCode levi_open_directory(const char* path);

/**
 * Open URL in default browser
 */
LeviErrorCode levi_open_url(const char* url);

/**
 * Get path to log file
 */
const char* levi_get_log_path(void);

/**
 * Run garbage collection on mod cache
 */
LeviErrorCode levi_cleanup_cache(void);

#ifdef __cplusplus
}
#endif