/**
 * LeviLinux Mod Menu System
 * In-game overlay for mod configuration and UI
 * Port of LeviLaunchroid's pl::modmenu to C++
 */
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <shared_mutex>

namespace levi::mod::modmenu {

// ─────────────────────────────────────────────────────────────────────────────
// CONFIG TYPES
// ─────────────────────────────────────────────────────────────────────────────

enum class ConfigType {
    Toggle,         // Boolean checkbox
    SliderInt,      // Integer slider
    SliderFloat,    // Float slider
    Dropdown,       // Dropdown menu
    Radio,          // Radio button group
    TextInput,      // Text input field
    Color,          // Color picker (#RRGGBB)
};

// ─────────────────────────────────────────────────────────────────────────────
// BUTTON
// ─────────────────────────────────────────────────────────────────────────────

enum class ButtonBehavior {
    Click,          // Single click/tap
    Hold,           // Hold to activate
    Toggle,         // Toggle on/off
};

enum class ButtonStylePreset {
    Default,        // System default style
    Accent,         // Accent color
    Danger,         // Red/warning style
    Success,        // Green/success style
};

struct ButtonStyle {
    ButtonStylePreset preset = ButtonStylePreset::Default;
    uint32_t bg_color = 0xFF222222;       // Background color (ARGB)
    uint32_t active_bg_color = 0xFF4AE0A0; // Active background color
    uint32_t pressed_bg_color = 0x994AE0A0; // Pressed background color
    uint32_t text_color = 0xFFFFFFFF;      // Text color
    uint32_t active_text_color = 0xFF000000; // Active text color
    float size_scale_x = 1.0f;
    float size_scale_y = 1.0f;
};

enum class ButtonEvent {
    Down = 0,       // Button pressed
    Up = 1,         // Button released
    Click = 2,      // Click completed
    StateChanged = 3 // Toggle state changed
};

// ─────────────────────────────────────────────────────────────────────────────
// CONFIG ENTRY
// ─────────────────────────────────────────────────────────────────────────────

struct ConfigEntry {
    std::string key;
    std::string label;
    ConfigType type;
    std::string value;           // Current value as string
    std::string min_value;       // For sliders
    std::string max_value;       // For sliders
    std::string options;         // For radio: "Option1,Option2,Option3"
};

// ─────────────────────────────────────────────────────────────────────────────
// MODULE
// ─────────────────────────────────────────────────────────────────────────────

struct Module {
    std::string id;
    std::string name;
    std::string description;
    std::string mod_id;          // Owning mod's ID
    bool enabled = false;
    std::vector<ConfigEntry> config_entries;

    using ToggleCallback = std::function<void(const std::string& module_id, bool enabled)>;
    using ConfigChangedCallback = std::function<void(const std::string& module_id,
                                                    const std::string& key,
                                                    const std::string& value)>;
    ToggleCallback on_toggle;
    ConfigChangedCallback on_config_changed;
};

// ─────────────────────────────────────────────────────────────────────────────
// BUTTON DEFINITION
// ─────────────────────────────────────────────────────────────────────────────

struct ButtonDef {
    std::string id;
    std::string label;
    std::string module_id;       // Associated module
    std::string tooltip;
    ButtonBehavior behavior = ButtonBehavior::Click;
    ButtonStyle style;
    int android_key_code = 0;    // Android keycode for hardware buttons

    // Icon data (exactly one should be set)
    std::vector<uint8_t> png_icon;
    std::vector<uint8_t> webp_icon;
    std::string svg_icon;        // SVG string

    // Event callback
    using EventCallback = std::function<void(const std::string& button_id,
                                              ButtonEvent event, float value)>;
    EventCallback on_event;
};

// ─────────────────────────────────────────────────────────────────────────────
// REGISTRY API (ModMenu - global registry)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Global mod menu registry
 * Mods register their modules/buttons here
 */
class ModMenuRegistry {
public:
    static ModMenuRegistry& instance();

    /**
     * Register a module
     */
    bool registerModule(const Module& module);

    /**
     * Unregister a module by ID
     */
    bool unregisterModule(const std::string& module_id);

    /**
     * Register a button
     */
    bool registerButton(const ButtonDef& button);

    /**
     * Unregister a button by ID
     */
    bool unregisterButton(const std::string& button_id);

    /**
     * Toggle module enabled state
     * Calls the module's onToggle callback
     */
    bool toggleModule(const std::string& module_id, bool enabled);

    /**
     * Update a config entry value
     * Calls the module's onConfigChanged callback
     */
    bool updateConfig(const std::string& module_id, const std::string& key,
                      const std::string& value);

    /**
     * Send button event
     * Calls the button's onEvent callback
     */
    bool sendButtonEvent(const std::string& button_id, ButtonEvent event, float value);

    /**
     * Get all registered modules
     */
    std::vector<Module> getModules() const;

    /**
     * Get all registered buttons
     */
    std::vector<ButtonDef> getButtons() const;

    /**
     * Get modules for a specific mod
     */
    std::vector<Module> getModModules(const std::string& mod_id) const;

    /**
     * Clear all registrations
     */
    void clear();

    // ── Callbacks ──────────────────────────────────────────────────────────

    using ToggleCallback = std::function<void(const std::string& module_id, bool enabled)>;
    using ConfigChangedCallback = std::function<void(const std::string& module_id,
                                                     const std::string& key,
                                                     const std::string& value)>;
    using ButtonEventCallback = std::function<void(const std::string& button_id,
                                                    ButtonEvent event, float value)>;

    void setToggleCallback(ToggleCallback cb) { toggle_cb_ = std::move(cb); }
    void setConfigChangedCallback(ConfigChangedCallback cb) { config_cb_ = std::move(cb); }
    void setButtonEventCallback(ButtonEventCallback cb) { button_cb_ = std::move(cb); }

private:
    ModMenuRegistry() = default;
    ~ModMenuRegistry() = default;
    ModMenuRegistry(const ModMenuRegistry&) = delete;
    ModMenuRegistry& operator=(const ModMenuRegistry&) = delete;

    mutable std::shared_mutex mutex_;
    std::vector<Module> modules_;
    std::vector<ButtonDef> buttons_;
    ToggleCallback toggle_cb_;
    ConfigChangedCallback config_cb_;
    ButtonEventCallback button_cb_;
};

// ─────────────────────────────────────────────────────────────────────────────
// BUILDER (Fluent API for registering modules/buttons)
// ─────────────────────────────────────────────────────────────────────────────

class ModuleBuilder {
public:
    explicit ModuleBuilder(const std::string& id, const std::string& name);

    ModuleBuilder& modId(const std::string& mod_id);
    ModuleBuilder& description(const std::string& desc);
    ModuleBuilder& defaultEnabled(bool enabled);
    ModuleBuilder& onToggle(Module::ToggleCallback cb);
    ModuleBuilder& onConfigChanged(Module::ConfigChangedCallback cb);
    ModuleBuilder& config(const std::string& key, const std::string& label,
                          ConfigType type, const std::string& value,
                          const std::string& min = "", const std::string& max = "",
                          const std::string& options = "");

    bool registerModule();

private:
    Module module_;
};

class ButtonBuilder {
public:
    explicit ButtonBuilder(const std::string& id, const std::string& label);

    ButtonBuilder& moduleId(const std::string& module_id);
    ButtonBuilder& tooltip(const std::string& tip);
    ButtonBuilder& behavior(ButtonBehavior behavior);
    ButtonBuilder& stylePreset(ButtonStylePreset preset);
    ButtonBuilder& styleColors(uint32_t bg, uint32_t active, uint32_t pressed);
    ButtonBuilder& textColor(uint32_t text);
    ButtonBuilder& activeTextColor(uint32_t text);
    ButtonBuilder& sizeScale(float x, float y);
    ButtonBuilder& androidKeyCode(int code);
    ButtonBuilder& pngIcon(const uint8_t* data, size_t size);
    ButtonBuilder& webpIcon(const uint8_t* data, size_t size);
    ButtonBuilder& svgIcon(const std::string& svg);
    ButtonBuilder& onEvent(ButtonDef::EventCallback cb);

    bool registerButton();

private:
    ButtonDef button_;
};

} // namespace levi::mod::modmenu