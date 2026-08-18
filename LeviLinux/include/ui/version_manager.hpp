#ifndef VERSION_MANAGER_HPP
#define VERSION_MANAGER_HPP

#include <QObject>
#include <QString>
#include <QVector>
#include <QFile>
#include <QDir>
#include <memory>

class CoreEngine;

struct GameVersion {
    QString id;
    QString name;
    QString type;
    QString releaseTime;
    QString versionFile;
    QString mainClass;
    QString librariesPath;
    QString nativesPath;
    QString assetsPath;
    bool isInstalled = false;
    QString installPath;
    qint64 fileSize = 0;
    QString sha1;
};

class VersionManager : public QObject {
    Q_OBJECT

public:
    explicit VersionManager(CoreEngine& engine, QObject* parent = nullptr);
    ~VersionManager();

    bool initialize();
    QVector<GameVersion> getAvailableVersions() const;
    QVector<GameVersion> getInstalledVersions() const;
    bool installVersion(const QString& versionId, const QString& installPath);
    bool uninstallVersion(const QString& versionId);
    bool launchVersion(const QString& versionId, const QStringList& arguments = {});
    GameVersion getVersionInfo(const QString& versionId) const;
    void refreshVersions();

    static QString getDefaultGameDir();
    static QString getVersionsDir();
    static QString getAssetsDir();

signals:
    void versionInstalled(const QString& versionId);
    void versionUninstalled(const QString& versionId);
    void installProgress(int progress, const QString& status);
    void errorOccurred(const QString& message);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // VERSION_MANAGER_HPP