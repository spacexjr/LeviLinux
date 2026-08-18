#include "ui/config_manager.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QMap>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir configDir(configPath);
    if (!configDir.exists("LeviLinux")) {
        configDir.mkpath("LeviLinux");
    }
    configFilePath = configDir.filePath("LeviLinux/config.json");
}

ConfigManager::~ConfigManager() {
    if (settings) {
        delete settings;
        settings = nullptr;
    }
}

void ConfigManager::initialize(const QString& organization, const QString& application) {
    settings = new QSettings(QSettings::UserScope, organization, application);
    setupDefaults();
}

QVariant ConfigManager::get(const QString& key, const QVariant& defaultValue) const {
    if (!settings) {
        return defaultValue;
    }
    return settings->value(key, defaultValue);
}

void ConfigManager::set(const QString& key, const QVariant& value) {
    if (settings) {
        settings->setValue(key, value);
    }
}

void ConfigManager::remove(const QString& key) {
    if (settings) {
        settings->remove(key);
    }
}

bool ConfigManager::contains(const QString& key) const {
    return settings && settings->contains(key);
}

QString ConfigManager::getGameDirectory() const {
    return get("gameDirectory", QString()).toString();
}

void ConfigManager::setGameDirectory(const QString& path) {
    set("gameDirectory", path);
}

QString ConfigManager::getJavaPath() const {
    return get("javaPath", "java").toString();
}

void ConfigManager::setJavaPath(const QString& path) {
    set("javaPath", path);
}

QString ConfigManager::getGraphicsAPI() const {
    return get("graphicsAPI", "auto").toString();
}

void ConfigManager::setGraphicsAPI(const QString& api) {
    set("graphicsAPI", api);
}

QString ConfigManager::getAudioAPI() const {
    return get("audioAPI", "auto").toString();
}

void ConfigManager::setAudioAPI(const QString& api) {
    set("audioAPI", api);
}

QString ConfigManager::getInputAPI() const {
    return get("inputAPI", "auto").toString();
}

void ConfigManager::setInputAPI(const QString& api) {
    set("inputAPI", api);
}

int ConfigManager::getWindowWidth() const {
    return get("windowWidth", 1000).toInt();
}

void ConfigManager::setWindowWidth(int width) {
    set("windowWidth", width);
}

int ConfigManager::getWindowHeight() const {
    return get("windowHeight", 700).toInt();
}

void ConfigManager::setWindowHeight(int height) {
    set("windowHeight", height);
}

bool ConfigManager::getFullscreen() const {
    return get("fullscreen", false).toBool();
}

void ConfigManager::setFullscreen(bool fullscreen) {
    set("fullscreen", fullscreen);
}

int ConfigManager::getMemoryAllocation() const {
    return get("memoryAllocation", 2048).toInt();
}

void ConfigManager::setMemoryAllocation(int mb) {
    set("memoryAllocation", mb);
}

QStringList ConfigManager::getJvmArguments() const {
    return get("jvmArguments", QStringList()).toStringList();
}

void ConfigManager::setJvmArguments(const QStringList& args) {
    set("jvmArguments", args);
}

bool ConfigManager::getAutoUpdate() const {
    return get("autoUpdate", true).toBool();
}

void ConfigManager::setAutoUpdate(bool enabled) {
    set("autoUpdate", enabled);
}

QString ConfigManager::getLanguage() const {
    return get("language", "en").toString();
}

void ConfigManager::setLanguage(const QString& lang) {
    set("language", lang);
}

void ConfigManager::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    QJsonObject::iterator it = obj.begin();
    while (it != obj.end()) {
        settings->setValue(it.key(), it.value().toVariant());
        ++it;
    }
}

void ConfigManager::saveToFile(const QString& filePath) const {
    if (!settings) {
        return;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    
    QMap<QString, QVariant> allSettings = settings->allKeys().toMap();
    QJsonObject obj;
    
    const QList<QByteArray> keys = settings->allKeys();
    for (const QByteArray& key : keys) {
        obj[QString::fromLocal8Bit(key)] = QJsonValue::fromVariant(settings->value(key).toVariant());
    }
    
    QJsonDocument doc(obj);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void ConfigManager::resetToDefaults() {
    if (!settings) {
        return;
    }
    
    QJsonObject::const_iterator it = defaults.constBegin();
    while (it != defaults.constEnd()) {
        settings->setValue(it.key(), it.value().toVariant());
        ++it;
    }
}

void ConfigManager::setupDefaults() {
    if (!settings) {
        return;
    }
    
    defaults = QJsonObject{
        {"gameDirectory", QJsonValue()},
        {"javaPath", QJsonValue("java")},
        {"graphicsAPI", QJsonValue("auto")},
        {"audioAPI", QJsonValue("auto")},
        {"inputAPI", QJsonValue("auto")},
        {"windowWidth", QJsonValue(1000)},
        {"windowHeight", QJsonValue(700)},
        {"fullscreen", QJsonValue(false)},
        {"memoryAllocation", QJsonValue(2048)},
        {"jvmArguments", QJsonValue(QJsonArray())},
        {"autoUpdate", QJsonValue(true)},
        {"language", QJsonValue("en")},
        {"maxFramerate", QJsonValue(60)},
        {"renderDistance", QJsonValue(8)},
        {"volume", QJsonValue(80)},
        {"bufferSize", QJsonValue(1024)}
    };
}