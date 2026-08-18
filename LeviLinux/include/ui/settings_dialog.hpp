#ifndef SETTINGS_DIALOG_HPP
#define SETTINGS_DIALOG_HPP

#include <QDialog>
#include <QSettings>

class CoreEngine;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(CoreEngine& engine, QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void onSaveSettings();
    void onResetDefaults();
    void onBrowseGameDir();
    void onBrowseJavaPath();
    void onGraphicsAPIChanged(int index);
    void onAudioAPIChanged(int index);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void populateGraphicsAPIs();
    void populateAudioAPIs();

    Ui::SettingsDialog* ui;
    CoreEngine& engine;
    QSettings settings;
};

#endif // SETTINGS_DIALOG_HPP