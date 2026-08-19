/**
 * LeviLinux Game Launcher
 * Orchestrates the full launch pipeline:
 *   Mods → Preloader → Auth → mcpelauncher-client
 */
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <atomic>
#include <thread>

namespace levi {

struct LaunchConfig;
struct LeviAuthTokens;
struct LeviSettings;

class GameLauncher {
public:
    struct Config {
        std::string version_id;
        std::string data_dir;           // ~/.local/share/mcpelauncher/versions/<ver>/
        std::string preloader_path;     // Path to libpreloader.so
        std::string client_path;        // Path to mcpelauncher-client
        std::string mods_cache_dir;     // ~/.cache/LeviLinux/mods/<ver>/
        std::string extra_args;
        std::string extra_env;          // KEY=VALUE\nKEY2=VALUE2
        bool disable_mods = false;
        int gpu_choice = 0;             // 0=auto, 1=iGPU, 2=dGPU
    };

    struct State {
        std::atomic<bool> running{false};
        std::atomic<int> pid{0};
        std::atomic<int> exit_code{0};
        std::atomic<bool> closing{false};
        std::string current_output;
    };

    enum LogLevel {
        LOG_DEBUG = 0,
        LOG_INFO = 1,
        LOG_WARN = 2,
        LOG_ERROR = 3,
    };

    using LogCallback = std::function<void(LogLevel level, const std::string& message)>;
    using ProgressCallback = std::function<void(float progress, const std::string& status)>;
    using GameStateCallback = std::function<void(int state, int exit_code)>;

    GameLauncher();
    ~GameLauncher();

    GameLauncher(const GameLauncher&) = delete;
    GameLauncher& operator=(const GameLauncher&) = delete;

    /**
     * Launch the game
     * @param config Launch configuration
     * @param log_cb Callback for game output logs
     * @param state_cb Callback for state changes
     */
    bool launch(const Config& config,
                LogCallback log_cb = nullptr,
                GameStateCallback state_cb = nullptr);

    /**
     * Check if game is running
     */
    bool isRunning() const { return state_.running.load(); }

    /**
     * Get game process PID (0 if not running)
     */
    int getPid() const { return state_.pid.load(); }

    /**
     * Force terminate game process
     */
    bool terminate();

    /**
     * Wait for game to exit
     * @param timeout_ms Max wait time (-1 = infinite)
     * @return Exit code, or -1 on timeout
     */
    int waitForExit(int timeout_ms = -1);

    /**
     * Get the last exit code
     */
    int getExitCode() const { return state_.exit_code.load(); }

    /**
     * Get current game output (buffered)
     */
    std::string getOutput() const { return state_.current_output; }

    /**
     * Apply renderer environment variables
     * Based on Trinity's game_launcher.cpp renderer settings
     */
    static void applyRendererEnv(int gpu_choice, bool force_vibrants,
                                  bool old_intel, bool nvidia_optimus,
                                  bool zink, bool glvk_fps, bool black_screen);

    /**
     * Validate client binary exists
     */
    static bool validateClient(const std::string& client_path, std::string& error);

    /**
     * Build mcpelauncher-client command line arguments
     */
    static std::vector<std::string> buildArgs(const Config& config);

    /**
     * Parse extra environment variables string (KEY=VAL\nKEY2=VAL2)
     */
    static std::vector<std::pair<std::string, std::string>>
    parseExtraEnv(const std::string& extra_env);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    Config config_;
    State state_;
    LogCallback log_cb_;
    GameStateCallback state_cb_;
    std::thread output_thread_;

    void readProcessOutput();
    void onGameOutput(const std::string& output);
    bool shouldCloseGame(const std::string& output);
};

} // namespace levi