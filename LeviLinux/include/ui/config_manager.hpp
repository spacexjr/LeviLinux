#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <QSettings>
#include <QString>
#include <QVariant>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

class ConfigManager {
public:
    static ConfigManager& instance();
    
    void initialize(const QString& organization = "LeviLinux", const QString& application = "LeviLinux");
    
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void set(const QString& key, const QVariant& value);
    void remove(const QString& key);
    bool contains(const QString& key) const;
    
    QString getGameDirectory() const;
    void setGameDirectory(const QString& path);
    
    QString getJavaPath() const;
    void setJavaPath(const QString& path);
    
    QString getGraphicsAPI() const;
    void setGraphicsAPI(const QString& api);
    
    QString getAudioAPI() const;
    void setAudioAPI(const QString& api);
    
    QString getInputAPI() const;
    void setInputAPI(const QString& api);
    
    int getWindowWidth() const;
    void setWindowWidth(int width);
    
    int getWindowHeight() const;
    void setWindowHeight(int height);
    
    bool getFullscreen() const;
    void setFullscreen(bool fullscreen);
    
    int getMemoryAllocation() const;
    void setMemoryAllocation(int mb);
    
    QStringList getJvmArguments() const;
    void setJvmArguments(const QStringList& args);
    
    bool getAutoUpdate() const;
    void setAutoUpdate(bool enabled);
    
    QString getLanguage() const;
    void setLanguage(const QString& lang);
    
    void loadFromFile(const QString& filePath);
    void saveToFile(const QString& filePath) const;
    void resetToDefaults();

private:
    ConfigManager();
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    mutable QSettings* settings = nullptr;
    QString configFilePath;
    QJsonObject defaults;
    
    void setupDefaults();
};

#endif // CONFIG_MANAGER_HPP