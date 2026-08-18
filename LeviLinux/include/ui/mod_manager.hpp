#ifndef MOD_MANAGER_HPP
#define MOD_MANAGER_HPP

#include <QObject>
#include <QString>
#include <QVector>
#include <QList>
#include <memory>

class CoreEngine;

struct ModInfo {
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    QString filePath;
    QString modType;
    bool isEnabled = false;
    bool isLoaded = false;
    bool isInstalled = false;
    QStringList dependencies;
    QString minecraftVersion;
    qint64 fileSize = 0;
    QString sha1;
};

class ModManager : public QObject {
    Q_OBJECT

public:
    explicit ModManager(CoreEngine& engine, QObject* parent = nullptr);
    ~ModManager();

    bool initialize();
    QVector<ModInfo> getAvailableMods() const;
    QVector<ModInfo> getLoadedMods() const;
    QVector<ModInfo> getEnabledMods() const;
    bool installMod(const QString& modPath);
    bool uninstallMod(const QString& modId);
    bool enableMod(const QString& modId);
    bool disableMod(const QString& modId);
    bool loadMod(const QString& modId);
    bool unloadMod(const QString& modId);
    ModInfo getModInfo(const QString& modId) const;
    void refreshMods();
    void scanModsDirectory(const QString& directory);

signals:
    void modInstalled(const QString& modId);
    void modUninstalled(const QString& modId);
    void modEnabled(const QString& modId);
    void modDisabled(const QString& modId);
    void modLoaded(const QString& modId);
    void modUnloaded(const QString& modId);
    void errorOccurred(const QString& message);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // MOD_MANAGER_HPP