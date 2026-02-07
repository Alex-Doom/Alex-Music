// SettingsDialog.cpp
#include "SettingsDialog.h"
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Настройки");
    setModal(true);
    resize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Группа настроек воспроизведения
    QGroupBox* playbackGroup = new QGroupBox("Настройки воспроизведения");
    QVBoxLayout* playbackLayout = new QVBoxLayout;

    autoPlayNextCheckBox = new QCheckBox("Автоматически воспроизводить следующий трек");
    playbackLayout->addWidget(autoPlayNextCheckBox);

    defaultVolumeSpinBox = new QSpinBox;
    defaultVolumeSpinBox->setRange(0, 100);
    defaultVolumeSpinBox->setSuffix("%");
    defaultVolumeSpinBox->setValue(70);
    QHBoxLayout* volumeLayout = new QHBoxLayout;
    volumeLayout->addWidget(new QLabel("Громкость по умолчанию:"));
    volumeLayout->addWidget(defaultVolumeSpinBox);
    volumeLayout->addStretch();
    playbackLayout->addLayout(volumeLayout);

    playbackGroup->setLayout(playbackLayout);
    mainLayout->addWidget(playbackGroup);

    // Группа обработки битых треков
    QGroupBox* badTracksGroup = new QGroupBox("Обработка повреждённых треков");
    QVBoxLayout* badTracksLayout = new QVBoxLayout;

    skipBadTracksCheckBox = new QCheckBox("Всегда пропускать повреждённые треки");
    badTracksLayout->addWidget(skipBadTracksCheckBox);

    autoSkipThresholdSpinBox = new QSpinBox;
    autoSkipThresholdSpinBox->setRange(1, 30);
    autoSkipThresholdSpinBox->setSuffix(" сек");
    autoSkipThresholdSpinBox->setValue(5);
    QHBoxLayout* thresholdLayout = new QHBoxLayout;
    thresholdLayout->addWidget(new QLabel("Порог пропуска (если трек короче):"));
    thresholdLayout->addWidget(autoSkipThresholdSpinBox);
    thresholdLayout->addStretch();
    badTracksLayout->addLayout(thresholdLayout);

    badTracksGroup->setLayout(badTracksLayout);
    mainLayout->addWidget(badTracksGroup);

    // Группа интерфейса
    QGroupBox* interfaceGroup = new QGroupBox("Интерфейс");
    QVBoxLayout* interfaceLayout = new QVBoxLayout;

    showNotificationsCheckBox = new QCheckBox("Показывать уведомления");
    interfaceLayout->addWidget(showNotificationsCheckBox);

    interfaceGroup->setLayout(interfaceLayout);
    mainLayout->addWidget(interfaceGroup);

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    saveButton = new QPushButton("Сохранить");
    cancelButton = new QPushButton("Отмена");
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Подключаем сигналы
    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveButton, &QPushButton::clicked, this, &SettingsDialog::settingsChanged);
}

// Геттеры
bool SettingsDialog::alwaysSkipBadTracks() const { return skipBadTracksCheckBox->isChecked(); }
bool SettingsDialog::autoPlayNext() const { return autoPlayNextCheckBox->isChecked(); }
int SettingsDialog::autoSkipThreshold() const { return autoSkipThresholdSpinBox->value(); }
bool SettingsDialog::showNotifications() const { return showNotificationsCheckBox->isChecked(); }
int SettingsDialog::defaultVolume() const { return defaultVolumeSpinBox->value(); }

// Сеттеры
void SettingsDialog::setAlwaysSkipBadTracks(bool skip) { skipBadTracksCheckBox->setChecked(skip); }
void SettingsDialog::setAutoPlayNext(bool autoPlay) { autoPlayNextCheckBox->setChecked(autoPlay); }
void SettingsDialog::setAutoSkipThreshold(int seconds) { autoSkipThresholdSpinBox->setValue(seconds); }
void SettingsDialog::setShowNotifications(bool show) { showNotificationsCheckBox->setChecked(show); }
void SettingsDialog::setDefaultVolume(int volume) { defaultVolumeSpinBox->setValue(volume); }
