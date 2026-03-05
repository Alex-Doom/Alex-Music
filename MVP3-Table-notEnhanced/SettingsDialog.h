// SettingsDialog.h
#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Геттеры
    bool alwaysSkipBadTracks() const;
    bool autoPlayNext() const;
    int autoSkipThreshold() const; // в секундах
    bool showNotifications() const;
    int defaultVolume() const;

    // Сеттеры
    void setAlwaysSkipBadTracks(bool skip);
    void setAutoPlayNext(bool autoPlay);
    void setAutoSkipThreshold(int seconds);
    void setShowNotifications(bool show);
    void setDefaultVolume(int volume);

signals:
    void settingsChanged();

private:
    QCheckBox* skipBadTracksCheckBox;
    QCheckBox* autoPlayNextCheckBox;
    QCheckBox* showNotificationsCheckBox;
    QSpinBox* autoSkipThresholdSpinBox;
    QSpinBox* defaultVolumeSpinBox;
    QPushButton* saveButton;
    QPushButton* cancelButton;
};
