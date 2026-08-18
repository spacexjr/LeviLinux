#include "ui/version_manager.hpp"
#include "core/core_engine.hpp"
#include "ui/config_manager.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>
#include <QCryptographicHash>
#include <QDateTime>

class VersionManager::Impl {
public:
    CoreEngine& engine;
    ConfigManager& config;
    QVector<GameVersion> availableVersions;
    QVector<GameVersion> installedVersions;
    QNetworkAccessManager* networkManager = nullptr;
    
    Impl(CoreEngine& e) : engine(e), config(ConfigManager::instance()) {}
    ~Impl() { if (networkManager) delete networkManager; }
};

VersionManager::VersionManager(CoreEngine& engine) : pImpl(std::make_unique<Impl>(engine)) {
    pImpl->networkManager = new QNetworkAccessManager();
}

VersionManager::~VersionManager() = default;

bool VersionManager::initialize() {
    refreshVersions();
    return true;
}

QVector<GameVersion> VersionManager::getAvailableVersions() const {
    return pImpl->availableVersions;
}

QVector<GameVersion> VersionManager::getInstalledVersions() const {
    return pImpl->installedVersions;
}

bool VersionManager::installVersion(const QString& versionId, const QString& installPath) {
    Q_UNUSED(versionId);
    Q_UNUSED(installPath);
    emit installProgress(100, "Installation complete (simulated)");
    return true;
}

bool VersionManager::uninstallVersion(const QString& versionId) {
    Q_UNUSED(versionId);
    emit versionUninstalled(versionId);
    return true;
}

bool VersionManager::launchVersion(const QString& versionId, const QStringList& arguments) {
    Q_UNUSED(arguments);
    emit versionInstalled(versionId);
    return true;
}

GameVersion VersionManager::getVersionInfo(const QString& versionId) const {
    for (const auto& version : pImpl->installedVersions) {
        if (version.id == versionId) {
            return version;
        }
    }
    return GameVersion();
}

void VersionManager::refreshVersions() {
    pImpl->installedVersions.clear();
    
    QString gameDir = ConfigManager::instance().getGameDirectory();
    if (gameDir.isEmpty()) {
        gameDir = QStringLiteral("%1/.levilinux").arg(QDir::homePath());
    }
    
    QString versionsPath = gameDir + "/versions";
    QDir versionsDir(versionsPath);
    
    if (!versionsDir.exists()) {
        versionsDir.mkpath(".");
    }
    
    QDirIterator dirIter(versionsDir, QDir::Files);
    while (dirIter.hasNext()) {
        QString filePath = dirIter.next();
        if (filePath.endsWith(".json")) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                file.close();
                
                QJsonDocument doc = QJsonDocument::fromJson(data);
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    GameVersion version;
                    version.id = obj.contains("id") ? obj["id"].toString() : QFileInfo(filePath).baseName();
                    version.name = version.id;
                    version.type = obj.contains("type") ? obj["type"].toString() : "release";
                    version.releaseTime = obj.contains("releaseTime") ? obj["releaseTime"].toString() : "";
                    version.mainClass = obj.contains("minecraftArguments") ? 
                        obj["minecraftArguments"].toString() : "net.minecraft.client.main.Main";
                    version.versionFile = filePath;
                    version.installPath = versionsDir.absolutePath();
                    version.isInstalled = true;
                    
                    if (obj.contains("time") && !obj["time"].toString().isEmpty()) {
                        version.releaseTime = obj["time"].toString();
                    }
                    
                    QJsonObject jars = obj.value("libraries").toObject();
                    for (auto it = jars.begin(); it != jars.end(); ++it) {
                        QJsonObject lib = it.value().toObject();
                        if (lib.contains("downloads")) {
                            QJsonObject downloads = lib["downloads"].toObject();
                            if (downloads.contains("artifact")) {
                                QJsonObject artifact = downloads["artifact"].toObject();
                                QString path = artifact.contains("path") ? artifact["path"].toString() : "";
                                if (!path.isEmpty()) {
                                    version.librariesPath += path + ";";
                                }
                            }
                        }
                    }
                    
                    if (version.librariesPath.isEmpty()) {
                        version.librariesPath = gameDir + "/versions/" + version.id;
                    }
                    
                    if (obj.contains("id")) {
                        version.id = obj["id"].toString();
                    }
                    
                    if (obj.contains("downloads")) {
                        QJsonObject downloads = obj["downloads"].toObject();
                        if (downloads.contains("client")) {
                            QJsonObject client = downloads["client"].toObject();
                            version.versionFile = client.contains("url") ? client["url"].toString() : "";
                            version.sha1 = client.contains("sha1") ? client["sha1"].toString() : "";
                            version.fileSize = client.contains("size") ? client["size"].toObject()["total"].toVariant().toLongLong() : 0;
                        }
                    }
                    
                    pImpl->installedVersions.append(version);
                }
            }
        }
    }
    
    if (pImpl->installedVersions.isEmpty()) {
        GameVersion defaultVersion;
        defaultVersion.id = "1.20.1";
        defaultVersion.name = "Minecraft 1.20.1";
        defaultVersion.type = "release";
        defaultVersion.releaseTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        defaultVersion.isInstalled = false;
        defaultVersion.installPath = gameDir + "/versions/1.20.1";
        defaultVersion.mainClass = "net.minecraft.client.main.Main";
        defaultVersion.librariesPath = gameDir + "/libraries";
        defaultVersion.assetsPath = gameDir + "/assets";
        pImpl->installedVersions.append(defaultVersion);
    }
}

QString VersionManager::getDefaultGameDir() {
    return QStringLiteral("%1/.levilinux").arg(QDir::homePath());
}

QString VersionManager::getVersionsDir() {
    return ConfigManager::instance().getGameDirectory() + "/versions";
}

QString VersionManager::getAssetsDir() {
    return ConfigManager::instance().getGameDirectory() + "/assets";
}