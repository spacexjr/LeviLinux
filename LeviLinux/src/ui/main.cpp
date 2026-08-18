#include "core/core_engine.hpp"
#include "ui/main_window.hpp"
#include "ui/settings_dialog.hpp"
#include "ui/version_manager.hpp"
#include "ui/mod_manager.hpp"
#include "ui/config_manager.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <iostream>
#include <signal.h>

#ifndef TEST_CORE_ENGINE_MAIN

static volatile bool running = true;

static void signalHandler(int signal) {
    running = false;
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
}

void setupSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LeviLinux");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LeviLinux");
    app.setStyle("Fusion");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("LeviLinux - Modular Minecraft Launcher for Linux");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Enable debug output");
    parser.addOption(debugOption);
    
    QCommandLineOption languageOption(QStringList() << "l" << "language", "Application language", "en");
    parser.addOption(languageOption);
    
    QCommandLineOption safeModeOption(QStringList() << "s" << "safe", "Safe mode - disable mods");
    parser.addOption(safeModeOption);
    
    parser.process(app);
    
    ConfigManager& config = ConfigManager::instance();
    config.initialize("LeviLinux", "LeviLinux");
    
    if (parser.isSet(debugOption)) {
        qDebug() << "Debug mode enabled";
        config.set("debug", true);
    }
    
    if (parser.isSet(languageOption)) {
        config.setLanguage(parser.value(languageOption));
    }
    
    setupSignalHandlers();
    
    std::cout << "LeviLinux Launcher v" << qPrintable(app.applicationVersion()) << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[Main] Creating CoreEngine..." << std::endl;
    std::cout.flush();
    
    CoreEngine engine;
    std::cout << "[Main] CoreEngine created" << std::endl;
    std::cout.flush();
    
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize core engine" << std::endl;
        std::cout << "[Main] Failed to initialize core engine" << std::endl;
        return 1;
    }
    
    std::cout << "Platform Info:" << std::endl;
    std::cout << engine.getPlatformInfo() << std::endl;
    
    QString gameDir = config.getGameDirectory();
    if (gameDir.isEmpty()) {
        QString defaultDir = QStringLiteral("%1/.levilinux").arg(QDir::homePath());
        config.setGameDirectory(defaultDir);
        gameDir = defaultDir;
    }
    
    QDir gameDirPath(gameDir);
    if (!gameDirPath.exists()) {
        gameDirPath.mkpath(".");
    }
    
    QDir modsDir(gameDir + "/mods");
    if (!modsDir.exists()) {
        modsDir.mkpath(".");
    }
    
    QDir versionsDir(gameDir + "/versions");
    if (!versionsDir.exists()) {
        versionsDir.mkpath(".");
    }
    
    std::cout << "[Main] Game directory: " << gameDir.toStdString() << std::endl;
    std::cout << "[Main] Mods directory: " << modsDir.path().toStdString() << std::endl;
    std::cout << "[Main] Versions directory: " << versionsDir.path().toStdString() << std::endl;
    
    if (!parser.isSet(safeModeOption)) {
        std::cout << "[Main] Loading core libraries..." << std::endl;
        std::vector<std::string> searchPaths = {
            gameDir.toStdString() + "/lib",
            "./lib",
            gameDir.toStdString() + "/versions",
            "/usr/lib",
            "/usr/local/lib"
        };
        
        bool librariesLoaded = false;
        for (const auto& path : searchPaths) {
            if (engine.loadCoreLibraries(path)) {
                std::cout << "Successfully loaded core libraries from: " << path << std::endl;
                librariesLoaded = true;
                break;
            }
        }
        
        if (!librariesLoaded) {
            std::cout << "Note: Core libraries not found in common paths" << std::endl;
        }
        
        std::cout << "[Main] Loading mods..." << std::endl;
    } else {
        std::cout << "[Main] Safe mode enabled - mods disabled" << std::endl;
    }
    
    MainWindow mainWindow(engine);
    mainWindow.show();
    
    std::cout << "[Main] UI launched successfully" << std::endl;
    std::cout.flush();
    
    int result = app.exec();
    
    std::cout << "\n[Main] Shutting down..." << std::endl;
    engine.cleanup();
    
    return result;
}

#else

#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    std::cout << "Testing Core Engine..." << std::endl;
    
    CoreEngine engine;
    if (engine.initialize()) {
        std::cout << "Core engine initialized successfully" << std::endl;
        std::cout << engine.getPlatformInfo() << std::endl;
        engine.cleanup();
        std::cout << "All tests passed!" << std::endl;
        return 0;
    }
    
    std::cerr << "Core engine initialization failed" << std::endl;
    return 1;
}

#endif // TEST_CORE_ENGINE_MAIN