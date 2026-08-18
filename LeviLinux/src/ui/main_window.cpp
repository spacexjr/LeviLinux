#include "ui/main_window.hpp"
#include "ui/settings_dialog.hpp"
#include "ui/version_manager.hpp"
#include "ui/mod_manager.hpp"
#include "ui/config_manager.hpp"
#include "core/core_engine.hpp"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QThread>
#include <QTimer>
#include <QCloseEvent>
#include <QSettings>
#include <QStyle>
#include <QScrollBar>
#include <QDateTime>
#include <iostream>

namespace Ui {
class MainWindow {
public:
    QWidget* centralWidget = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    QSplitter* mainSplitter = nullptr;
    QWidget* leftPanel = nullptr;
    QVBoxLayout* leftLayout = nullptr;
    QGroupBox* versionGroup = nullptr;
    QVBoxLayout* versionLayout = nullptr;
    QTableView* versionTable = nullptr;
    QHBoxLayout* versionButtons = nullptr;
    QPushButton* refreshBtn = nullptr;
    QPushButton* launchBtn = nullptr;
    QPushButton* installBtn = nullptr;
    QWidget* rightPanel = nullptr;
    QVBoxLayout* rightLayout = nullptr;
    QGroupBox* logGroup = nullptr;
    QVBoxLayout* logLayout = nullptr;
    QTextEdit* logOutput = nullptr;
    QProgressBar* progressBar = nullptr;
    QStatusBar* statusBar = nullptr;
    QMenuBar* menuBar = nullptr;
    QMenu* fileMenu = nullptr;
    QMenu* viewMenu = nullptr;
    QMenu* toolsMenu = nullptr;
    QMenu* helpMenu = nullptr;
    QAction* settingsAction = nullptr;
    QAction* quitAction = nullptr;
    QAction* manageModsAction = nullptr;
    QAction* aboutAction = nullptr;
    
    void setupUi(QMainWindow* mainWindow) {
        mainWindow->setObjectName("MainWindow");
        mainWindow->resize(1000, 700);
        mainWindow->setWindowTitle("LeviLinux - Minecraft Launcher");
        
        centralWidget = new QWidget(mainWindow);
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(5, 5, 5, 5);
        mainLayout->setSpacing(5);
        
        mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
        mainLayout->addWidget(mainSplitter);
        
        leftPanel = new QWidget(mainSplitter);
        leftLayout = new QVBoxLayout(leftPanel);
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(5);
        
        versionGroup = new QGroupBox("Game Versions", leftPanel);
        versionLayout = new QVBoxLayout(versionGroup);
        
        versionTable = new QTableView(versionGroup);
        versionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        versionTable->setSelectionMode(QAbstractItemView::SingleSelection);
        versionTable->setAlternatingRowColors(true);
        versionTable->setSortingEnabled(true);
        versionTable->horizontalHeader()->setStretchLastSection(true);
        versionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        versionTable->verticalHeader()->setVisible(false);
        versionLayout->addWidget(versionTable);
        
        versionButtons = new QHBoxLayout();
        refreshBtn = new QPushButton("Refresh", versionGroup);
        launchBtn = new QPushButton("Launch", versionGroup);
        launchBtn->setEnabled(false);
        installBtn = new QPushButton("Install Version", versionGroup);
        versionButtons->addWidget(refreshBtn);
        versionButtons->addWidget(launchBtn);
        versionButtons->addWidget(installBtn);
        versionButtons->addStretch();
        versionLayout->addLayout(versionButtons);
        
        leftLayout->addWidget(versionGroup);
        
        rightPanel = new QWidget(mainSplitter);
        rightLayout = new QVBoxLayout(rightPanel);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(5);
        
        logGroup = new QGroupBox("Log Output", rightPanel);
        logLayout = new QVBoxLayout(logGroup);
        
        logOutput = new QTextEdit(logGroup);
        logOutput->setReadOnly(true);
        logOutput->setFont(QFont("Monospace", 9));
        logLayout->addWidget(logOutput);
        
        progressBar = new QProgressBar(logGroup);
        progressBar->setVisible(false);
        progressBar->setTextVisible(true);
        logLayout->addWidget(progressBar);
        
        rightLayout->addWidget(logGroup);
        
        mainSplitter->addWidget(leftPanel);
        mainSplitter->addWidget(rightPanel);
        mainSplitter->setSizes({600, 400});
        
        mainWindow->setCentralWidget(centralWidget);
        
        statusBar = new QStatusBar(mainWindow);
        statusBar->showMessage("Ready");
        mainWindow->setStatusBar(statusBar);
        
        menuBar = new QMenuBar(mainWindow);
        fileMenu = menuBar->addMenu("&File");
        viewMenu = menuBar->addMenu("&View");
        toolsMenu = menuBar->addMenu("&Tools");
        helpMenu = menuBar->addMenu("&Help");
        
        settingsAction = new QAction("&Settings...", mainWindow);
        settingsAction->setShortcut(QKeySequence::Preferences);
        fileMenu->addAction(settingsAction);
        fileMenu->addSeparator();
        quitAction = new QAction("&Quit", mainWindow);
        quitAction->setShortcut(QKeySequence::Quit);
        fileMenu->addAction(quitAction);
        
        manageModsAction = new QAction("&Manage Mods...", mainWindow);
        toolsMenu->addAction(manageModsAction);
        
        aboutAction = new QAction("&About LeviLinux", mainWindow);
        helpMenu->addAction(aboutAction);
        
        mainWindow->setMenuBar(menuBar);
        
        QMetaObject::connectSlotsByName(mainWindow);
    }
};
}

MainWindow::MainWindow(CoreEngine& engine, QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), engine(engine), versionModel(new QStandardItemModel(this)) {
    ui->setupUi(this);
    setupUI();
    setupConnections();
    loadGameVersions();
    
    ConfigManager& config = ConfigManager::instance();
    resize(config.getWindowWidth(), config.getWindowHeight());
    if (config.getFullscreen()) {
        showFullScreen();
    }
}

MainWindow::~MainWindow() {
    ConfigManager& config = ConfigManager::instance();
    config.setWindowWidth(width());
    config.setWindowHeight(height());
    config.setFullscreen(isFullScreen());
    delete ui;
}

void MainWindow::setupUI() {
    versionModel->setHorizontalHeaderLabels({"Version", "Type", "Release Date", "Status", "Path"});
    ui->versionTable->setModel(versionModel);
    ui->versionTable->setColumnWidth(0, 150);
    ui->versionTable->setColumnWidth(1, 100);
    ui->versionTable->setColumnWidth(2, 150);
    ui->versionTable->setColumnWidth(3, 100);
    ui->versionTable->setColumnWidth(4, 300);
    
    appendLog("LeviLinux Launcher started");
    appendLog("Platform: " + QString::fromStdString(engine.getPlatformInfo()));
}

void MainWindow::setupConnections() {
    connect(ui->refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshVersions);
    connect(ui->launchBtn, &QPushButton::clicked, this, &MainWindow::onLaunchGame);
    connect(ui->installBtn, &QPushButton::clicked, this, &MainWindow::onInstallMod);
    connect(ui->versionTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onVersionSelected);
    connect(ui->settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
    connect(ui->manageModsAction, &QAction::triggered, this, &MainWindow::onManageMods);
    connect(ui->quitAction, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About LeviLinux",
            "LeviLinux - Modular Minecraft Launcher for Linux\n\n"
            "Version: 1.0.0\n"
            "Built with Qt6\n"
            "Core Engine: Trinity-Launcher adaptation\n"
            "Library Loader: LeviLaunchroid adaptation\n"
            "UI: LeviLauncher adaptation\n\n"
            "Native Linux support with Android API shims\n"
            "x86_64 and aarch64 architecture support");
    });
}

void MainWindow::loadGameVersions() {
    versionModel->removeRows(0, versionModel->rowCount());
    
    VersionManager versionManager(engine);
    versionManager.initialize();
    auto versions = versionManager.getInstalledVersions();
    
    for (const auto& version : versions) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(version.name.isEmpty() ? version.id : version.name));
        row.append(new QStandardItem(version.type));
        row.append(new QStandardItem(version.releaseTime));
        row.append(new QStandardItem(version.isInstalled ? "Installed" : "Not Installed"));
        row.append(new QStandardItem(version.installPath));
        for (auto* item : row) {
            item->setEditable(false);
            item->setData(version.id, Qt::UserRole);
        }
        versionModel->appendRow(row);
    }
    
    if (versions.isEmpty()) {
        appendLog("No game versions found. Use 'Install Version' to add one.");
    } else {
        appendLog(QString("Loaded %1 game version(s)").arg(versions.size()));
    }
}

void MainWindow::updateVersionList() {
    loadGameVersions();
}

void MainWindow::onRefreshVersions() {
    appendLog("Refreshing version list...");
    loadGameVersions();
    appendLog("Version list refreshed");
}

void MainWindow::onLaunchGame() {
    QModelIndex index = ui->versionTable->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "No Version Selected", "Please select a game version to launch.");
        return;
    }
    
    QString versionId = index.data(Qt::UserRole).toString();
    if (versionId.isEmpty()) {
        QMessageBox::warning(this, "Invalid Version", "Selected version has no valid ID.");
        return;
    }
    
    appendLog("Launching version: " + versionId);
    ui->progressBar->setVisible(true);
    ui->progressBar->setRange(0, 0);
    ui->launchBtn->setEnabled(false);
    
    VersionManager versionManager(engine);
    versionManager.initialize();
    
    QStringList args;
    ConfigManager& config = ConfigManager::instance();
    args << config.getJvmArguments();
    
    if (versionManager.launchVersion(versionId, args)) {
        appendLog("Game launched successfully");
    } else {
        appendLog("Failed to launch game");
        QMessageBox::critical(this, "Launch Failed", "Failed to launch the selected game version.");
    }
    
    ui->progressBar->setVisible(false);
    ui->launchBtn->setEnabled(true);
}

void MainWindow::onOpenSettings() {
    SettingsDialog dialog(engine, this);
    if (dialog.exec() == QDialog::Accepted) {
        appendLog("Settings saved");
        loadGameVersions();
    }
}

void MainWindow::onInstallMod() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select Mod File", QString(),
        "Mod Files (*.so *.jar *.zip);;All Files (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    appendLog("Installing mod: " + filePath);
    
    ModManager modManager(engine);
    modManager.initialize();
    
    if (modManager.installMod(filePath)) {
        appendLog("Mod installed successfully");
        QMessageBox::information(this, "Mod Installed", "The mod has been installed successfully.");
    } else {
        appendLog("Failed to install mod");
        QMessageBox::critical(this, "Install Failed", "Failed to install the selected mod.");
    }
}

void MainWindow::onManageMods() {
    QMessageBox::information(this, "Manage Mods", "Mod management dialog will be implemented here.");
}

void MainWindow::onVersionSelected(const QModelIndex& index) {
    ui->launchBtn->setEnabled(index.isValid());
}

void MainWindow::onLogMessage(const QString& message) {
    appendLog(message);
}

void MainWindow::appendLog(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    ui->logOutput->append(timestamp + message);
    QScrollBar* scrollBar = ui->logOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    ConfigManager& config = ConfigManager::instance();
    config.setWindowWidth(width());
    config.setWindowHeight(height());
    config.setFullscreen(isFullScreen());
    event->accept();
}