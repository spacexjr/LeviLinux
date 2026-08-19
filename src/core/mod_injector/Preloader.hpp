/**
 * LeviLinux Preloader
 * Core of the mod injection system
 * Hooks into mcpelauncher-client's ELF loader to inject mods before JNI_OnLoad
 */
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <dlfcn.h>

// JNI types (minimal forward declarations)
struct JavaVM_;
struct JNIEnv_;

namespace levi::mod {

struct ModEntry;

// ─────────────────────────────────────────────────────────────────────────────
// PRELOADER ENGINE
// ─────────────────────────────────────────────────────────────────────────────

class Preloader {
public:
    // Function pointers exported from libpreloader.so
    using PreloaderInitFn        = bool (*)(const ModEntry*, size_t);
    using PreloaderShutdownFn    = void (*)();
    using PreloaderJNILoadFn     = void (*)(JavaVM_*, void*);
    using PreloaderLoadModFn     = bool (*)(const ModEntry*);
    using PreloaderEnableFn      = bool (*)(const char*);
    using PreloaderDisableFn     = bool (*)(const char*);
    using PreloaderUnloadFn      = bool (*)(const char*);

    struct Config {
        std::string preloader_lib_path;      // Path to libpreloader.so
        std::string mods_cache_dir;          // ~/.cache/LeviLinux/mods/<version>/
        std::string game_data_dir;           // ~/.local/share/mcpelauncher/versions/<ver>/
        bool enable_hooking = true;          // Enable PLH-based hooks
        bool log_to_file = true;             // Also write logs to file
    };

    struct LoadContext {
        int total_mods = 0;
        int loaded_mods = 0;
        int failed_mods = 0;
        std::vector<std::string> errors;
        std::vector<std::string> loaded_mod_ids;
    };

    using LogCallback = std::function<void(int level, const std::string& message)>;

    Preloader();
    ~Preloader();

    Preloader(const Preloader&) = delete;
    Preloader& operator=(const Preloader&) = delete;
    Preloader(Preloader&&) noexcept;
    Preloader& operator=(Preloader&&) noexcept;

    /**
     * Initialize preloader engine
     * Loads libpreloader.so and resolves all function pointers
     * Call this after verifying the library exists
     */
    bool initialize(const Config& config, LogCallback log_cb = nullptr);

    /**
     * Shutdown preloader engine
     * Unloads libpreloader.so and clears all state
     */
    void shutdown();

    /**
     * Load mods before game starts
     * Called from LaunchService before mcpelauncher-client process
     */
    LoadContext loadMods(const std::vector<ModEntry>& mods);

    /**
     * Called from JNI_OnLoad hook inside mcpelauncher-client process
     * This is the actual injection point
     */
    void onJNILoad(JavaVM_* vm, void* reserved);

    /**
     * Enable all loaded mods
     */
    bool enableAllMods();

    /**
     * Disable all loaded mods
     */
    bool disableAllMods();

    /**
     * Unload all mods
     */
    void unloadAllMods();

    /**
     * Check if preloader is initialized
     */
    bool isInitialized() const { return initialized_; }

    /**
     * Get the shared library handle
     */
    void* getHandle() const { return handle_; }

private:
    bool initialized_ = false;
    void* handle_ = nullptr;
    Config config_;
    LogCallback log_cb_;
    std::vector<ModEntry> mods_;

    // Resolved function pointers from libpreloader.so
    PreloaderInitFn init_fn_ = nullptr;
    PreloaderShutdownFn shutdown_fn_ = nullptr;
    PreloaderJNILoadFn jni_load_fn_ = nullptr;
    PreloaderLoadModFn load_mod_fn_ = nullptr;
    PreloaderEnableFn enable_fn_ = nullptr;
    PreloaderDisableFn disable_fn_ = nullptr;
    PreloaderUnloadFn unload_fn_ = nullptr;

    bool resolveFunctions();
    void logError(const std::string& msg);
    void logInfo(const std::string& msg);
};

// ─────────────────────────────────────────────────────────────────────────────
// HOOK MANAGER (PLH / Capstone based)
// ─────────────────────────────────────────────────────────────────────────────

class HookManager {
public:
    struct HookEntry {
        std::string name;
        void* original_address = nullptr;
        void* hook_address = nullptr;
        void* trampoline = nullptr;
        bool enabled = false;
    };

    HookManager();
    ~HookManager();

    HookManager(const HookManager&) = delete;
    HookManager& operator=(const HookManager&) = delete;

    /**
     * Initialize hook manager with Capstone disassembler
     */
    bool initialize();

    /**
     * Hook a function by symbol name in a loaded library
     * @param lib_handle dlopen handle of the library
     * @param symbol_name Symbol to hook
     * @param hook_func New function to call
     * @param original_func Receives pointer to original function
     */
    bool hookSymbol(void* lib_handle, const char* symbol_name,
                    void* hook_func, void** original_func);

    /**
     * Hook a function by address
     */
    bool hookAddress(void* address, void* hook_func, void** original_func);

    /**
     * Remove a hook by name
     */
    bool unhook(const std::string& name);

    /**
     * Enable/disable a hook
     */
    bool setEnabled(const std::string& name, bool enabled);

    /**
     * Remove all hooks
     */
    void unhookAll();

    /**
     * Get all active hooks
     */
    std::vector<HookEntry> getHooks() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// ─────────────────────────────────────────────────────────────────────────────
// SYMBOL PATCHER (Runtime symbol replacement)
// ─────────────────────────────────────────────────────────────────────────────

class SymbolPatcher {
public:
    struct PatchedSymbol {
        std::string library;
        std::string symbol;
        void* original_address = nullptr;
        void* patched_address = nullptr;
        bool active = false;
    };

    SymbolPatcher();
    ~SymbolPatcher();

    SymbolPatcher(const SymbolPatcher&) = delete;
    SymbolPatcher& operator=(const SymbolPatcher&) = delete;

    /**
     * Patch a symbol in a loaded library
     * @param lib_handle dlopen handle
     * @param symbol_name Symbol to patch
     * @param new_impl New implementation
     */
    bool patchSymbol(void* lib_handle, const char* symbol_name, void* new_impl);

    /**
     * Patch a symbol by address (direct memory write)
     */
    bool patchAddress(void* address, void* new_impl, size_t size);

    /**
     * Restore a patched symbol
     */
    bool restoreSymbol(const std::string& library, const std::string& symbol);

    /**
     * Restore all patched symbols
     */
    void restoreAll();

    /**
     * Get list of patched symbols
     */
    std::vector<PatchedSymbol> getPatchedSymbols() const;

    /**
     * Apply GOT/PLT hook (Global Offset Table)
     */
    bool patchGOT(void* lib_handle, const char* symbol_name, void* new_impl);

    /**
     * Apply inline hook (code patching with trampoline)
     */
    bool patchInline(void* address, void* new_impl, void** original_out);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// ─────────────────────────────────────────────────────────────────────────────
// ANDROID SHIM LAYER (Bionic → glibc translation)
// ─────────────────────────────────────────────────────────────────────────────

class AndroidShim {
public:
    struct Config {
        bool enable_egl_shim = true;        // EGL → GLX/Vulkan
        bool enable_opensles_shim = true;   // OpenSL ES → PipeWire/PulseAudio
        bool enable_asset_shim = true;      // AAssetManager → physfs
        bool enable_jni_shim = true;        // JNI → libjnivm
        bool enable_pthread_shim = true;    // pthread TLS
        bool enable_log_shim = true;        // __android_log_* → printf/stderr
    };

    AndroidShim();
    ~AndroidShim();

    /**
     * Initialize shim layer
     */
    bool initialize(const Config& config);

    /**
     * Apply shims before loading libminecraftpe.so
     * Called from mcpelauncher-linker's symbol resolution
     */
    void applyShims(void* linker_handle);

    /**
     * Get function pointer for a shimmed Android API
     */
    void* getShim(const char* android_symbol);

    /**
     * Apply EGL shims (redirect EGL calls to GLFW/SDL3)
     */
    void applyEGLShims(void* egl_handle);

    /**
     * Apply audio shims (redirect OpenSL ES to PipeWire/PulseAudio)
     */
    void applyAudioShims(void* opensles_handle);

    /**
     * Apply asset shims (redirect AAssetManager to filesystem)
     */
    void applyAssetShims(void* assetmgr_handle);

    /**
     * Check if a symbol should be shimmed
     */
    bool isShimmable(const char* symbol) const;

    /**
     * Get shim mapping table
     */
    struct ShimMapping {
        const char* android_symbol;
        const char* linux_symbol;
        void* linux_address;
    };
    std::vector<ShimMapping> getShimMappings() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace levi::mod