/**
 * LeviLinux NativeMod Interface
 * Port of LeviLaunchroid's pl::mod::NativeMod to C++
 * All mods implement this interface
 */
#pragma once

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <cstdint>

namespace levi::mod {

class NativeMod;
class ModContext;

// ─────────────────────────────────────────────────────────────────────────────
// MOD CONTEXT (Provided by preloader to mods)
// ─────────────────────────────────────────────────────────────────────────────

class ModContext {
public:
    virtual ~ModContext() = default;

    /**
     * Get a symbol from the game process (libminecraftpe.so)
     * Useful for hooking game functions
     */
    virtual void* getSymbol(const char* name) = 0;

    /**
     * Get a symbol from a loaded mod
     */
    virtual void* getModSymbol(const char* mod_id, const char* symbol) = 0;

    /**
     * Get the path to this mod's root directory
     */
    virtual const char* getModRootPath() = 0;

    /**
     * Get the path to this mod's config directory
     */
    virtual const char* getModConfigPath() = 0;

    /**
     * Get the path to this mod's data directory
     */
    virtual const char* getModDataPath() = 0;

    /**
     * Get the current Minecraft version string
     */
    virtual const char* getMinecraftVersion() = 0;

    /**
     * Log a message from this mod
     */
    virtual void logInfo(const char* msg) = 0;
    virtual void logWarn(const char* msg) = 0;
    virtual void logError(const char* msg) = 0;

    /**
     * Hook a game function
     * @param symbol_name Symbol to hook
     * @param hook_func New function to call
     * @param original_out Receives pointer to call original
     */
    virtual bool hookSymbol(const char* symbol_name, void* hook_func, void** original_out) = 0;

    /**
     * Unhook a previously hooked function
     */
    virtual bool unhookSymbol(const char* symbol_name) = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// NATIVE MOD INTERFACE
// ─────────────────────────────────────────────────────────────────────────────

class NativeMod {
public:
    virtual ~NativeMod() = default;

    // ── Lifecycle ──────────────────────────────────────────────────────────

    /**
     * Called when the mod's .so is loaded
     * @return false to abort loading
     */
    virtual bool load(ModContext& ctx) { return true; }

    /**
     * Called after load() when the mod should activate
     * Register hooks, mod menu entries, etc. here
     * @return false to indicate failure (mod will be disabled)
     */
    virtual bool enable(ModContext& ctx) { return true; }

    /**
     * Called when the mod is being disabled
     * Remove hooks, clean up resources here
     * @return false to indicate failure
     */
    virtual bool disable(ModContext& ctx) { return true; }

    /**
     * Called when the mod is being unloaded
     * Final cleanup, save state, etc.
     * @return false to indicate failure
     */
    virtual bool unload(ModContext& ctx) { return true; }

    // ── Metadata ───────────────────────────────────────────────────────────

    /**
     * Unique mod identifier (e.g., "fps_counter", "zoom_mod")
     */
    virtual const char* getId() const = 0;

    /**
     * Human-readable mod name (e.g., "FPS Counter")
     */
    virtual const char* getName() const = 0;

    /**
     * Mod version string (e.g., "1.0.0")
     */
    virtual const char* getVersion() const = 0;

    /**
     * Mod author (optional, can return nullptr)
     */
    virtual const char* getAuthor() const { return nullptr; }

    /**
     * Mod description (optional, can return nullptr)
     */
    virtual const char* getDescription() const { return nullptr; }

    // ── Hooks ──────────────────────────────────────────────────────────────

    /**
     * Called when a hooked function is invoked
     * Override in mod to intercept game calls
     */
    virtual void onHookCalled(const char* hook_name, void* original_fn,
                              void** args, void* return_value) {}

    // ── Utility ────────────────────────────────────────────────────────────

    /**
     * Get the currently loading mod (thread-local)
     * Only valid during load/enable/disable/unload calls
     */
    static NativeMod* current();

    /**
     * Set the current mod (called by preloader)
     */
    static void setCurrent(NativeMod* mod);

private:
    static thread_local NativeMod* current_;
};

// ─────────────────────────────────────────────────────────────────────────────
// REGISTRATION MACRO
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Register a NativeMod class as the mod entry point.
 * Must be placed at file scope in the mod's .so source.
 *
 * Usage:
 *   class MyMod : public pl::mod::NativeMod { ... };
 *   MyMod g_mod;
 *   PL_REGISTER_MOD(MyMod, g_mod)
 */
#define PL_REGISTER_MOD(ModClass, instance) \
    extern "C" __attribute__((visibility("default"))) \
    levi::mod::NativeMod* pl_mod_create() { \
        return &(instance); \
    } \
    extern "C" __attribute__((visibility("default"))) \
    void pl_mod_destroy(levi::mod::NativeMod* mod) { \
        (void)mod; /* singleton, do not delete */ \
    } \
    extern "C" __attribute__((visibility("default"))) \
    const char* pl_mod_id() { \
        static ModClass mod_instance; \
        return mod_instance.getId(); \
    }

// ─────────────────────────────────────────────────────────────────────────────
// CONCRETE MOD CONTEXT (Used by preloader)
// ─────────────────────────────────────────────────────────────────────────────

class ConcreteModContext : public ModContext {
public:
    struct Params {
        std::string mod_id;
        std::string mod_root_path;
        std::string mod_config_path;
        std::string mod_data_path;
        std::string minecraft_version;
        void* game_symbol_resolver;  // For resolving game symbols
        void* mod_symbol_resolver;   // For resolving mod symbols
    };

    explicit ConcreteModContext(const Params& params);

    void* getSymbol(const char* name) override;
    void* getModSymbol(const char* mod_id, const char* symbol) override;
    const char* getModRootPath() override;
    const char* getModConfigPath() override;
    const char* getModDataPath() override;
    const char* getMinecraftVersion() override;
    void logInfo(const char* msg) override;
    void logWarn(const char* msg) override;
    void logError(const char* msg) override;
    bool hookSymbol(const char* symbol_name, void* hook_func, void** original_out) override;
    bool unhookSymbol(const char* symbol_name) override;

private:
    Params params_;
};

} // namespace levi::mod