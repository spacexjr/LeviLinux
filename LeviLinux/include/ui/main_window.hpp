#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <memory>

class CoreEngine;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(CoreEngine& engine, QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onRefreshVersions();
    void onLaunchGame();
    void onOpenSettings();
    void onInstallMod();
    void onManageMods();
    void onVersionSelected(const QModelIndex& index);
    void onLogMessage(const QString& message);

private:
    void setupUI();
    void setupConnections();
    void loadGameVersions();
    void updateVersionList();
    void appendLog(const QString& message);

    Ui::MainWindow* ui;
    CoreEngine& engine;
    QStandardItemModel* versionModel;
};

#endif // MAIN_WINDOW_HPP