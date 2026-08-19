/**
 * LeviLinux Bridge API - Shared Types
 * C-compatible types for UI ↔ Core communication
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ─────────────────────────────────────────────────────────────────────────────
// VERSION INFO
// ─────────────────────────────────────────────────────────────────────────────

#define LEVILINUX_BRIDGE_API_VERSION 1

typedef struct {
    int major;
    int minor;
    int patch;
    const char* git_commit;
    const char* build_date;
} LeviLinuxVersion;

// ─────────────────────────────────────────────────────────────────────────────
// ERROR CODES
// ─────────────────────────────────────────────────────────────────────────────

typedef enum {
    LEVI_OK = 0,
    LEVI_ERROR_GENERIC = -1,
    LEVI_ERROR_INVALID_ARG = -2,
    LEVI_ERROR_NOT_FOUND = -3,
    LEVI_ERROR_PERMISSION = -4,
    LEVI_ERROR_NO_MEMORY = -5,
    LEVI_ERROR_IO = -6,
    LEVI_ERROR_VERSION_MISMATCH = -7,
    LEVI_ERROR_NOT_INITIALIZED = -8,
    LEVI_ERROR_ALREADY_RUNNING = -9,
    LEVI_ERROR_AUTH_FAILED = -10,
    LEVI_ERROR_MOD_LOAD_FAILED = -11,
    LEVI_ERROR_INCOMPATIBLE_VERSION = -12,
    LEVI_ERROR_MCPelaUNCHER_MISSING = -13,
    LEVI_ERROR_PRELOADER_MISSING = -14,
} LeviErrorCode;

// ─────────────────────────────────────────────────────────────────────────────
// CALLBACKS
// ─────────────────────────────────────────────────────────────────────────────

/** Log callback - called from core with game output */
typedef void (*LeviLogCallback)(int level, const char* message, void* user_data);

/** Progress callback - for long operations (extraction, download) */
typedef void (*LeviProgressCallback)(float progress, const char* status, void* user_data);

/** Mod event callback */
typedef void (*LeviModEventCallback)(const char* mod_id, int event_type, void* user_data);

/** Game state callback */
typedef void (*LeviGameStateCallback)(int state, int exit_code, void* user_data);

/** Auth callback */
typedef void (*LeviAuthCallback)(bool success, const char* xuid, const char* gamertag, void* user_data);

#define LEVI_LOG_DEBUG 0
#define LEVI_LOG_INFO 1
#define LEVI_LOG_WARN 2
#define LEVI_LOG_ERROR 3

#define LEVI_MOD_EVENT_LOADED 0
#define LEVI_MOD_EVENT_ENABLED 1
#define LEVI_MOD_EVENT_DISABLED 2
#define LEVI_MOD_EVENT_UNLOADED 3
#define LEVI_MOD_EVENT_FAILED 4

#define LEVI_GAME_STATE_STARTING 0
#define LEVI_GAME_STATE_RUNNING 1
#define LEVI_GAME_STATE_STOPPED 2
#define LEVI_GAME_STATE_CRASHED 3

// ─────────────────────────────────────────────────────────────────────────────
// VERSION / INSTALLATION
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    char id[256];              // e.g., "1.21.0"
    char name[256];            // Display name
    char path[1024];           // Full path to version directory
    char architecture[32];     // "x86_64", "aarch64", "x86"
    bool valid;                // Has libminecraftpe.so
    bool installed;            // Fully extracted
    int64_t size_bytes;        // Installation size
    int64_t last_played;       // Unix timestamp
} LeviVersionInfo;

typedef struct {
    LeviVersionInfo* versions;
    size_t count;
} LeviVersionList;

// ─────────────────────────────────────────────────────────────────────────────
// MODS
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    char id[256];
    char name[256];
    char version[64];
    char author[256];
    char description[1024];
    char entry_path[512];      // Relative to mod root (e.g., "libmod.so")
    char root_path[1024];      // Full path to mod directory
    char config_path[1024];    // Full path to config directory
    char icon_path[1024];      // Optional icon
    char minecraft_versions[1024]; // Comma-separated version patterns
    bool enabled;
    bool compatible;           // Compatible with current MC version
    bool has_config;
    int config_file_count;
    int load_order;            // 0 = first loaded
} LeviModInfo;

typedef struct {
    LeviModInfo* mods;
    size_t count;
} LeviModList;

// ─────────────────────────────────────────────────────────────────────────────
// AUTHENTICATION (MSA / Xbox Live)
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    char xuid[64];
    char gamertag[64];
    char access_token[2048];
    char identity_token[2048];
    int64_t expires_at;        // Unix timestamp
    char display_name[64];
    char avatar_url[512];
} LeviAuthTokens;

typedef struct {
    char email[256];
    char xuid[64];
    char gamertag[64];
    char avatar_url[512];
    bool is_valid;
} LeviAccountInfo;

// ─────────────────────────────────────────────────────────────────────────────
// LAUNCH CONFIGURATION
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    char version_id[256];
    char mods_enabled[4096];   // Comma-separated mod IDs
    char extra_env[4096];      // Extra env vars (KEY=VAL\n)
    char extra_args[4096];     // Extra command line args
    bool use_dgpu;
    bool force_vulkan;
    bool disable_mods;
    int gpu_choice;            // 0=auto, 1=iGPU, 2=dGPU
} LeviLaunchConfig;

// ─────────────────────────────────────────────────────────────────────────────
// SETTINGS
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    char versions_dir[1024];
    char mods_dir[1024];
    char cache_dir[1024];
    char logs_dir[1024];
    bool auto_update;
    bool minimize_on_launch;
    bool close_on_game_exit;
    int log_level;
    char language[16];
    // Renderer settings
    bool force_vibrants;
    bool old_intel;
    bool nvidia_optimus;
    bool zink;
    bool glvk_fps;
    bool black_screen_fix;
} LeviSettings;

// ─────────────────────────────────────────────────────────────────────────────
// CONTEXT / OPAQUE HANDLES
// ─────────────────────────────────────────────────────────────────────────────

typedef struct LeviContext LeviContext;

#ifdef __cplusplus
}
#endif