/**
 * LeviLinux Bridge API - Callback Function Types
 * Detailed callback signatures for async operations
 */
#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ─────────────────────────────────────────────────────────────────────────────
// DOWNLOAD / EXTRACTION PROGRESS
// ─────────────────────────────────────────────────────────────────────────────

typedef enum {
    LEVI_PROGRESS_STARTING = 0,
    LEVI_PROGRESS_DOWNLOADING = 1,
    LEVI_PROGRESS_EXTRACTING = 2,
    LEVI_PROGRESS_VERIFYING = 3,
    LEVI_PROGRESS_COMPLETE = 4,
    LEVI_PROGRESS_FAILED = 5,
    LEVI_PROGRESS_CANCELLED = 6,
} LeviProgressStage;

typedef struct {
    LeviProgressStage stage;
    float progress;           // 0.0 - 1.0
    int64_t bytes_downloaded;
    int64_t bytes_total;
    int64_t speed_bps;        // Bytes per second
    const char* current_file;
    const char* status_message;
} LeviProgressInfo;

typedef void (*LeviProgressCallbackV2)(const LeviProgressInfo* info, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// MOD INSTALLATION PROGRESS
// ─────────────────────────────────────────────────────────────────────────────

typedef enum {
    LEVI_MOD_STAGE_SCANNING = 0,
    LEVI_MOD_STAGE_VALIDATING = 1,
    LEVI_MOD_STAGE_COPYING = 2,
    LEVI_MOD_STAGE_REGISTERING = 3,
    LEVI_MOD_STAGE_COMPLETE = 4,
    LEVI_MOD_STAGE_FAILED = 5,
} LeviModStage;

typedef struct {
    LeviModStage stage;
    const char* mod_id;
    const char* mod_name;
    float progress;
    const char* status_message;
} LeviModProgressInfo;

typedef void (*LeviModProgressCallback)(const LeviModProgressInfo* info, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// AUTHENTICATION PROGRESS
// ─────────────────────────────────────────────────────────────────────────────

typedef enum {
    LEVI_AUTH_STAGE_STARTING = 0,
    LEVI_AUTH_STAGE_DEVICE_CODE = 1,      // User needs to enter code
    LEVI_AUTH_STAGE_WAITING_AUTH = 2,     // Waiting for user to authorize
    LEVI_AUTH_STAGE_EXCHANGING_TOKENS = 3,
    LEVI_AUTH_STAGE_FETCHING_PROFILE = 4,
    LEVI_AUTH_STAGE_COMPLETE = 5,
    LEVI_AUTH_STAGE_FAILED = 6,
    LEVI_AUTH_STAGE_CANCELLED = 7,
} LeviAuthStage;

typedef struct {
    LeviAuthStage stage;
    const char* device_code;        // For DEVICE_CODE stage
    const char* verification_uri;   // For DEVICE_CODE stage
    const char* user_code;          // For DEVICE_CODE stage
    int expires_in;                 // Seconds until code expires
    int interval;                   // Polling interval in seconds
    const char* status_message;
} LeviAuthProgressInfo;

typedef void (*LeviAuthProgressCallback)(const LeviAuthProgressInfo* info, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// GAME LAUNCH PROGRESS
// ─────────────────────────────────────────────────────────────────────────────

typedef enum {
    LEVI_LAUNCH_STAGE_PREPARING = 0,
    LEVI_LAUNCH_STAGE_LOADING_MODS = 1,
    LEVI_LAUNCH_STAGE_INIT_PRELOADER = 2,
    LEVI_LAUNCH_STAGE_AUTHENTICATING = 3,
    LEVI_LAUNCH_STAGE_STARTING_PROCESS = 4,
    LEVI_LAUNCH_STAGE_WAITING_WINDOW = 5,
    LEVI_LAUNCH_STAGE_RUNNING = 6,
    LEVI_LAUNCH_STAGE_FAILED = 7,
} LeviLaunchStage;

typedef struct {
    LeviLaunchStage stage;
    float progress;
    const char* status_message;
    int pid;                        // Process ID when started
} LeviLaunchProgressInfo;

typedef void (*LeviLaunchProgressCallback)(const LeviLaunchProgressInfo* info, void* user_data);

// ─────────────────────────────────────────────────────────────────────────────
// EXTENDED API WITH PROGRESS CALLBACKS
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Extract version with detailed progress
 */
LeviErrorCode levi_versions_extract_v2(const char* apk_path, const char* version_id,
                                        LeviProgressCallbackV2 progress_cb, void* user_data);

/**
 * Install mod with detailed progress
 */
LeviErrorCode levi_mods_install_v2(const char* mod_path,
                                    LeviModProgressCallback progress_cb, void* user_data);

/**
 * Login with detailed progress (for device code flow UI)
 */
LeviErrorCode levi_auth_login_v2(LeviAuthProgressCallback progress_cb,
                                  LeviAuthCallback complete_cb, void* user_data);

/**
 * Launch game with detailed progress
 */
LeviErrorCode levi_launch_game_v2(const LeviLaunchConfig* config,
                                   LeviLaunchProgressCallback progress_cb,
                                   LeviGameStateCallback state_cb, void* user_data);

#ifdef __cplusplus
}
#endif