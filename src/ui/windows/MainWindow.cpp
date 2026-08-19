#include "MainWindow.hpp"

// Need complete types for unique_ptr members in destructor
#include "../services/LaunchService.hpp"
#include "../services/VersionService.hpp"
#include "../services/ModService.hpp"
#include "../services/AuthService.hpp"
#include "../services/SettingsService.hpp"
#include "ModManagerDialog.hpp"
#include "AccountManagerDialog.hpp"
#include "SettingsDialog.hpp"
#include "LogConsole.hpp"

namespace levi {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {}
MainWindow::~MainWindow() = default;

void MainWindow::setCurrentVersionIndex(int index) { current_index_ = index; }

void MainWindow::closeEvent(QCloseEvent*) {}
void MainWindow::showEvent(QShowEvent*) {}

void MainWindow::refreshVersions() {}
void MainWindow::selectVersion(int) {}
void MainWindow::onVersionExtracted(const QString&) {}
void MainWindow::onVersionDeleted(const QString&) {}

void MainWindow::onPlayClicked() {}
void MainWindow::onGameStateChanged(int, int) {}

void MainWindow::openModManager() {}
void MainWindow::onModsChanged() {}

void MainWindow::openAccountManager() {}
void MainWindow::onAuthStateChanged(bool) {}

void MainWindow::openSettings() {}
void MainWindow::onSettingsChanged() {}

void MainWindow::openLogConsole() {}
void MainWindow::onGameLog(int, const QString&) {}

void MainWindow::updateDiscordActivity(bool, const QString&) {}

void MainWindow::setupUi() {}
void MainWindow::setupMenuBar() {}
void MainWindow::setupToolBar() {}
void MainWindow::setupStatusBar() {}
void MainWindow::setupConnections() {}
void MainWindow::loadTheme(const QString&) {}
void MainWindow::updatePlayButton() {}
void MainWindow::updateVersionSelector() {}
void MainWindow::restoreWindowState() {}
void MainWindow::saveWindowState() {}

} // namespace levi
