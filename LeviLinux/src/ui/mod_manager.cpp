#include "ui/mod_manager.hpp"
#include "core/core_engine.hpp"
#include "ui/config_manager.hpp"
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDebug>
#include <QFile>

class ModManager::Impl {
public:
    CoreEngine& engine;
    ConfigManager& config;
    QVector<ModInfo> loadedMods;
    QVector<ModInfo> installedMods;
    
    Impl(CoreEngine& e) : engine(e), config(ConfigManager::instance()) {}
};

ModManager::ModManager(CoreEngine& engine, QObject* parent)
    : QObject(parent), pImpl(std::make_unique<Impl>(engine)) {
}

ModManager::~ModManager() = default;

bool ModManager::initialize() {
    scanModsDirectory(ConfigManager::instance().getGameDirectory() + "/mods");
    return true;
}

QVector<ModInfo> ModManager::getAvailableMods() const {
    return pImpl->installedMods;
}

QVector<ModInfo> ModManager::getLoadedMods() const {
    return pImpl->loadedMods;
}

QVector<ModInfo> ModManager::getEnabledMods() const {
    QVector<ModInfo> enabled;
    for (const auto& mod : pImpl->installedMods) {
        if (mod.isEnabled) {
            enabled.append(mod);
        }
    }
    return enabled;
}

bool ModManager::installMod(const QString& modPath) {
    QFile source(modPath);
    if (!source.exists()) {
        emit errorOccurred("Mod file does not exist: " + modPath);
        return false;
    }
    
    QString gameDir = ConfigManager::instance().getGameDirectory();
    QString modsDir = gameDir + "/mods";
    
    QDir dir(modsDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString destPath = modsDir + "/" + QFileInfo(modPath).fileName();
    if (source.copy(destPath)) {
        ModInfo modInfo;
        modInfo.filePath = destPath;
        modInfo.id = QFileInfo(modPath).baseName();
        modInfo.name = modInfo.id;
        modInfo.version = "1.0.0";
        modInfo.isEnabled = true;
        modInfo.isInstalled = true;
        modInfo.fileSize = source.size();
        
        pImpl->installedMods.append(modInfo);
        emit modInstalled(modInfo.id);
        return true;
    }
    
    emit errorOccurred("Failed to copy mod file");
    return false;
}

bool ModManager::uninstallMod(const QString& modId) {
    for (int i = 0; i < pImpl->installedMods.size(); ++i) {
        if (pImpl->installedMods[i].id == modId) {
            QFile::remove(pImpl->installedMods[i].filePath);
            pImpl->installedMods.removeAt(i);
            emit modUninstalled(modId);
            return true;
        }
    }
    return false;
}

bool ModManager::enableMod(const QString& modId) {
    for (auto& mod : pImpl->installedMods) {
        if (mod.id == modId) {
            mod.isEnabled = true;
            emit modEnabled(modId);
            return true;
        }
    }
    return false;
}

bool ModManager::disableMod(const QString& modId) {
    for (auto& mod : pImpl->installedMods) {
        if (mod.id == modId) {
            mod.isEnabled = false;
            emit modDisabled(modId);
            return true;
        }
    }
    return false;
}

bool ModManager::loadMod(const QString& modId) {
    for (auto& mod : pImpl->installedMods) {
        if (mod.id == modId && mod.isEnabled) {
            mod.isLoaded = true;
            for (const auto& loadedMod : pImpl->loadedMods) {
                if (loadedMod.id == modId) {
                    return true;
                }
            }
            pImpl->loadedMods.append(mod);
            emit modLoaded(modId);
            return true;
        }
    }
    return false;
}

bool ModManager::unloadMod(const QString& modId) {
    for (int i = 0; i < pImpl->loadedMods.size(); ++i) {
        if (pImpl->loadedMods[i].id == modId) {
            pImpl->loadedMods[i].isLoaded = false;
            pImpl->loadedMods.removeAt(i);
            emit modUnloaded(modId);
            return true;
        }
    }
    return false;
}

ModInfo ModManager::getModInfo(const QString& modId) const {
    for (const auto& mod : pImpl->installedMods) {
        if (mod.id == modId) {
            return mod;
        }
    }
    return ModInfo();
}

void ModManager::refreshMods() {
    initialize();
}

void ModManager::scanModsDirectory(const QString& directory) {
    pImpl->installedMods.clear();
    pImpl->loadedMods.clear();
    
    QDir modsDir(directory);
    if (!modsDir.exists()) {
        return;
    }
    
    QStringList filters;
    filters << "*.jar" << "*.zip" << "*.so";
    modsDir.setNameFilters(filters);
    
    for (const QFileInfo& fileInfo : modsDir.entryInfoList(QDir::Files)) {
        ModInfo modInfo;
        modInfo.filePath = fileInfo.absoluteFilePath();
        modInfo.id = fileInfo.baseName();
        modInfo.name = fileInfo.baseName();
        modInfo.version = "1.0.0";
        modInfo.isEnabled = true;
        modInfo.isInstalled = true;
        modInfo.isLoaded = false;
        modInfo.fileSize = fileInfo.size();
        modInfo.modType = fileInfo.suffix();
        
        QString jsonPath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".json";
        QFile jsonFile(jsonPath);
        if (jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)) {
            QByteArray data = jsonFile.readAll();
            jsonFile.close();
            
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                modInfo.name = obj.value("name").toString(modInfo.name);
                modInfo.version = obj.value("version").toString(modInfo.version);
                modInfo.description = obj.value("description").toString();
                modInfo.author = obj.value("author").toString("Unknown");
                modInfo.modType = obj.value("type").toString(modInfo.modType);
                modInfo.minecraftVersion = obj.value("minecraft_version").toString();
                
                QJsonValue depsVal = obj.value("dependencies");
                if (depsVal.isArray()) {
                    QJsonArray deps = depsVal.toArray();
                    for (const auto& dep : deps) {
                        modInfo.dependencies.append(dep.toString());
                    }
                }
            }
        }
        
        pImpl->installedMods.append(modInfo);
    }
}