/**
 * LeviLinux Main Window
 * Primary launcher UI (ported from Trinity's LauncherWindow)
 */
#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>
#include <memory>

class QStackedWidget;
class QLabel;
class QPushButton;
class QComboBox;
class QProgressBar;
class QTextEdit;
class QSettings;
class QThread;

namespace levi {

class LaunchService;
class VersionService;
class ModService;
class AuthService;
class SettingsService;

// Forward declarations for UI widgets
class VersionSelector;
class ModManagerDialog;
class AccountManagerDialog;
class SettingsDialog;
class LogConsole;

class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(int currentVersionIndex READ currentVersionIndex WRITE setCurrentVersionIndex NOTIFY currentVersionChanged)

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    int currentVersionIndex() const { return current_index_; }
    void setCurrentVersionIndex(int index);

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

signals:
    // State changes
    void gameStateChanged(int state, int exit_code);
    void currentVersionChanged(int index);
    void versionsRefreshed();
    void modsRefreshed();
    void authStateChanged(bool logged_in);

    // UI events
    void logMessage(int level, const QString& message);
    void progressUpdate(float progress, const QString& status);

public slots:
    // Version management
    void refreshVersions();
    void selectVersion(int index);
    void onVersionExtracted(const QString& version_id);
    void onVersionDeleted(const QString& version_id);

    // Game launch
    void onPlayClicked();
    void onGameStateChanged(int state, int exit_code);

    // Mod management
    void openModManager();
    void onModsChanged();

    // Account management
    void openAccountManager();
    void onAuthStateChanged(bool logged_in);

    // Settings
    void openSettings();
    void onSettingsChanged();

    // Log
    void openLogConsole();
    void onGameLog(int level, const QString& message);

    // Discord Rich Presence
    void updateDiscordActivity(bool playing, const QString& version);

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void loadTheme(const QString& theme_path);
    void updatePlayButton();
    void updateVersionSelector();
    void restoreWindowState();
    void saveWindowState();

    // UI Components
    QStackedWidget* central_stack_;
    QWidget* home_page_;
    QWidget* launch_page_;
    
    // Home page widgets
    QComboBox* version_combo_;
    QPushButton* play_button_;
    QPushButton* mods_button_;
    QPushButton* accounts_button_;
    QPushButton* settings_button_;
    QPushButton* refresh_button_;
    QLabel* status_label_;
    QLabel* version_info_label_;
    QLabel* mod_count_label_;
    QProgressBar* launch_progress_;
    
    // Log console
    LogConsole* log_console_;
    
    // Dialogs (lazy-initialized)
    ModManagerDialog* mod_manager_dialog_;
    AccountManagerDialog* account_dialog_;
    SettingsDialog* settings_dialog_;
    
    // System tray
    QSystemTrayIcon* tray_icon_;
    
    // Services (owned)
    std::unique_ptr<LaunchService> launch_service_;
    std::unique_ptr<VersionService> version_service_;
    std::unique_ptr<ModService> mod_service_;
    std::unique_ptr<AuthService> auth_service_;
    std::unique_ptr<SettingsService> settings_service_;
    
    // State
    int current_index_ = -1;
    bool game_running_ = false;
    int game_pid_ = 0;
    
    // Settings
    QSettings* settings_;
};

} // namespace levi