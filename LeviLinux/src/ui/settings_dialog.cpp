#include "ui/settings_dialog.hpp"
#include "ui/config_manager.hpp"
#include "core/core_engine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QSize>

namespace Ui {
class SettingsDialog {
public:
    QTabWidget* tabWidget = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
    
    QWidget* generalTab = nullptr;
    QFormLayout* generalLayout = nullptr;
    QLineEdit* gameDirEdit = nullptr;
    QPushButton* browseGameDirBtn = nullptr;
    QLineEdit* javaPathEdit = nullptr;
    QPushButton* browseJavaPathBtn = nullptr;
    QSpinBox* memorySpinBox = nullptr;
    QLineEdit* jvmArgsEdit = nullptr;
    QCheckBox* autoUpdateCheckbox = nullptr;
    
    QWidget* graphicsTab = nullptr;
    QFormLayout* graphicsLayout = nullptr;
    QComboBox* graphicsAPICombo = nullptr;
    QSpinBox* maxFramerateSpin = nullptr;
    QCheckBox* fullscreenCheckbox = nullptr;
    QSpinBox* renderDistanceSpin = nullptr;
    
    QWidget* audioTab = nullptr;
    QFormLayout* audioLayout = nullptr;
    QComboBox* audioAPICombo = nullptr;
    QSpinBox* volumeSpin = nullptr;
    QSpinBox* bufferSizeSpin = nullptr;
    
    QWidget* controlTab = nullptr;
    
    void setupUi(QDialog* dialog) {
        dialog->setWindowTitle("Settings");
        dialog->resize(600, 400);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(dialog);
        
        tabWidget = new QTabWidget(dialog);
        mainLayout->addWidget(tabWidget);
        
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
        mainLayout->addWidget(buttonBox);
        
        setupGeneralTab();
        setupGraphicsTab();
        setupAudioTab();
        setupControlTab();
    }
    
private:
    void setupGeneralTab() {
        generalTab = new QWidget(tabWidget);
        tabWidget->addTab(generalTab, "General");
        
        generalLayout = new QFormLayout(generalTab);
        
        QLabel* gameDirLabel = new QLabel("Game Directory:", generalTab);
        gameDirEdit = new QLineEdit(generalTab);
        browseGameDirBtn = new QPushButton("Browse...", generalTab);
        
        QHBoxLayout* gameDirLayout = new QHBoxLayout();
        gameDirLayout->addWidget(gameDirEdit);
        gameDirLayout->addWidget(browseGameDirBtn);
        generalLayout->addRow(gameDirLabel, gameDirLayout);
        
        QLabel* javaPathLabel = new QLabel("Java Path:", generalTab);
        javaPathEdit = new QLineEdit(generalTab);
        browseJavaPathBtn = new QPushButton("Browse...", generalTab);
        
        QHBoxLayout* javaPathLayout = new QHBoxLayout();
        javaPathLayout->addWidget(javaPathEdit);
        javaPathLayout->addWidget(browseJavaPathBtn);
        generalLayout->addRow(javaPathLabel, javaPathLayout);
        
        QLabel* memoryLabel = new QLabel("Memory Allocation (MB):", generalTab);
        memorySpinBox = new QSpinBox(generalTab);
        memorySpinBox->setRange(512, 8192);
        memorySpinBox->setValue(2048);
        memorySpinBox->setSingleStep(256);
        generalLayout->addRow(memoryLabel, memorySpinBox);
        
        QLabel* jvmLabel = new QLabel("JVM Arguments:", generalTab);
        jvmArgsEdit = new QLineEdit(generalTab);
        jvmArgsEdit->setPlaceholderText("e.g. -Xmx2G -XX:+UseG1GC");
        generalLayout->addRow(jvmLabel, jvmArgsEdit);
        
        autoUpdateCheckbox = new QCheckBox("Enable Auto-Update", generalTab);
        autoUpdateCheckbox->setChecked(true);
        generalLayout->addRow(autoUpdateCheckbox);
    }
    
    void setupGraphicsTab() {
        graphicsTab = new QWidget(tabWidget);
        tabWidget->addTab(graphicsTab, "Graphics");
        
        graphicsLayout = new QFormLayout(graphicsTab);
        
        QLabel* graphicsLabel = new QLabel("Graphics API:", graphicsTab);
        graphicsAPICombo = new QComboBox(graphicsTab);
        populateGraphicsAPIs();
        graphicsLayout->addRow(graphicsLabel, graphicsAPICombo);
        
        QLabel* framerateLabel = new QLabel("Max Framerate:", graphicsTab);
        maxFramerateSpin = new QSpinBox(graphicsTab);
        maxFramerateSpin->setRange(30, 240);
        maxFramerateSpin->setValue(60);
        graphicsLayout->addRow(framerateLabel, maxFramerateSpin);
        
        QLabel* fullscreenLabel = new QLabel("Fullscreen:", graphicsTab);
        fullscreenCheckbox = new QCheckBox("Enable Fullscreen", graphicsTab);
        graphicsLayout->addRow(fullscreenLabel, fullscreenCheckbox);
        
        QLabel* renderLabel = new QLabel("Render Distance:", graphicsTab);
        renderDistanceSpin = new QSpinBox(graphicsTab);
        renderDistanceSpin->setRange(2, 32);
        renderDistanceSpin->setValue(8);
        graphicsLayout->addRow(renderLabel, renderDistanceSpin);
    }
    
    void populateGraphicsAPIs() {
        graphicsAPICombo->clear();
        graphicsAPICombo->addItem("OpenGL", "opengl");
        graphicsAPICombo->addItem("OpenGL ES 2.0", "opengles2");
        graphicsAPICombo->addItem("Vulkan (if available)", "vulkan");
        graphicsAPICombo->addItem("Auto Detect", "auto");
    }
    
    void setupAudioTab() {
        audioTab = new QWidget(tabWidget);
        tabWidget->addTab(audioTab, "Audio");
        
        audioLayout = new QFormLayout(audioTab);
        
        QLabel* audioLabel = new QLabel("Audio API:", audioTab);
        audioAPICombo = new QComboBox(audioTab);
        populateAudioAPIs();
        audioLayout->addRow(audioLabel, audioAPICombo);
        
        QLabel* volumeLabel = new QLabel("Master Volume:", audioTab);
        volumeSpin = new QSpinBox(audioTab);
        volumeSpin->setRange(0, 100);
        volumeSpin->setValue(80);
        audioLayout->addRow(volumeLabel, volumeSpin);
        
        QLabel* bufferLabel = new QLabel("Buffer Size:", audioTab);
        bufferSizeSpin = new QSpinBox(audioTab);
        bufferSizeSpin->setRange(256, 8192);
        bufferSizeSpin->setValue(1024);
        bufferSizeSpin->setSingleStep(256);
        audioLayout->addRow(bufferLabel, bufferSizeSpin);
    }
    
    void populateAudioAPIs() {
        audioAPICombo->clear();
        audioAPICombo->addItem("PulseAudio", "pulseaudio");
        audioAPICombo->addItem("ALSA", "alsa");
        audioAPICombo->addItem("PipeWire", "pipewire");
        audioAPICombo->addItem("Auto Detect", "auto");
    }
    
    void setupControlTab() {
        controlTab = new QWidget(tabWidget);
        tabWidget->addTab(controlTab, "Controls");
        
        QVBoxLayout* controlVBoxLayout = new QVBoxLayout(controlTab);
        
        QLabel* keyBindsLabel = new QLabel("Key Bindings: Coming Soon", controlTab);
        controlVBoxLayout->addWidget(keyBindsLabel);
        
        controlVBoxLayout->addStretch();
    }
};
}

SettingsDialog::SettingsDialog(CoreEngine& engine, QWidget* parent)
    : QDialog(parent), ui(new Ui::SettingsDialog), engine(engine), settings("LeviLinux", "LeviLinux") {
    ui->setupUi(this);
    loadSettings();
    
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onSaveSettings);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->browseGameDirBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseGameDir);
    connect(ui->browseJavaPathBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseJavaPath);
    connect(ui->graphicsAPICombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onGraphicsAPIChanged);
    connect(ui->audioAPICombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onAudioAPIChanged);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::onSaveSettings() {
    saveSettings();
    accept();
}

void SettingsDialog::onResetDefaults() {
    settings.clear();
    loadSettings();
}

void SettingsDialog::onBrowseGameDir() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Game Directory",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (!dir.isEmpty()) {
        ui->gameDirEdit->setText(dir);
    }
}

void SettingsDialog::onBrowseJavaPath() {
    QString file = QFileDialog::getOpenFileName(this, "Select Java Executable",
        QString(), "Java Executable (*.exe);;Any File (*)");
    if (!file.isEmpty()) {
        ui->javaPathEdit->setText(file);
    }
}

void SettingsDialog::onGraphicsAPIChanged(int index) {
    Q_UNUSED(index);
    QString api = ui->graphicsAPICombo->itemData(ui->graphicsAPICombo->currentIndex()).toString();
    engine.getPlatformAbstraction();
}

void SettingsDialog::onAudioAPIChanged(int index) {
    Q_UNUSED(index);
    QString api = ui->audioAPICombo->itemData(ui->audioAPICombo->currentIndex()).toString();
}

void SettingsDialog::loadSettings() {
    QString defaultDir = QStringLiteral("%1/.levilinux").arg(QDir::homePath());
    ui->gameDirEdit->setText(settings.value("gameDirectory", defaultDir).toString());
    ui->javaPathEdit->setText(settings.value("javaPath", "java").toString());
    ui->memorySpinBox->setValue(settings.value("memoryAllocation", 2048).toInt());
    ui->jvmArgsEdit->setText(settings.value("jvmArguments", "").toString());
    ui->autoUpdateCheckbox->setChecked(settings.value("autoUpdate", true).toBool());
    
    QString graphicsApi = settings.value("graphicsAPI", "auto").toString();
    int graphicsIndex = ui->graphicsAPICombo->findData(graphicsApi);
    if (graphicsIndex >= 0) {
        ui->graphicsAPICombo->setCurrentIndex(graphicsIndex);
    }
    ui->maxFramerateSpin->setValue(settings.value("maxFramerate", 60).toInt());
    ui->fullscreenCheckbox->setChecked(settings.value("fullscreen", false).toBool());
    ui->renderDistanceSpin->setValue(settings.value("renderDistance", 8).toInt());
    
    QString audioApi = settings.value("audioAPI", "auto").toString();
    int audioIndex = ui->audioAPICombo->findData(audioApi);
    if (audioIndex >= 0) {
        ui->audioAPICombo->setCurrentIndex(audioIndex);
    }
    ui->volumeSpin->setValue(settings.value("volume", 80).toInt());
    ui->bufferSizeSpin->setValue(settings.value("bufferSize", 1024).toInt());
}

void SettingsDialog::saveSettings() {
    settings.setValue("gameDirectory", ui->gameDirEdit->text());
    settings.setValue("javaPath", ui->javaPathEdit->text());
    settings.setValue("memoryAllocation", ui->memorySpinBox->value());
    settings.setValue("jvmArguments", ui->jvmArgsEdit->text());
    settings.setValue("autoUpdate", ui->autoUpdateCheckbox->isChecked());
    
    settings.setValue("graphicsAPI", ui->graphicsAPICombo->itemData(ui->graphicsAPICombo->currentIndex()).toString());
    settings.setValue("maxFramerate", ui->maxFramerateSpin->value());
    settings.setValue("fullscreen", ui->fullscreenCheckbox->isChecked());
    settings.setValue("renderDistance", ui->renderDistanceSpin->value());
    
    settings.setValue("audioAPI", ui->audioAPICombo->itemData(ui->audioAPICombo->currentIndex()).toString());
    settings.setValue("volume", ui->volumeSpin->value());
    settings.setValue("bufferSize", ui->bufferSizeSpin->value());
    
    settings.sync();
}