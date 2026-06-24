// SettingsDialog.cpp
#include "SettingsDialog.h"
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Настройки AlexMusic");
    setModal(true);
    resize(400, 280); // Уменьшаем высоту, убираем лишнее

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Заголовок
    QLabel* titleLabel = new QLabel("⚙ Настройки");
    titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; color: #0078d4; }");
    mainLayout->addWidget(titleLabel);

    // Группа настроек воспроизведения
    QGroupBox* playbackGroup = new QGroupBox("🎵 Воспроизведение");
    QVBoxLayout* playbackLayout = new QVBoxLayout;

    QHBoxLayout* volumeLayout = new QHBoxLayout;
    volumeLayout->addWidget(new QLabel("Громкость по умолчанию:"));
    defaultVolumeSpinBox = new QSpinBox;
    defaultVolumeSpinBox->setRange(0, 100);
    defaultVolumeSpinBox->setSuffix("%");
    defaultVolumeSpinBox->setValue(70);
    defaultVolumeSpinBox->setFixedWidth(80);
    volumeLayout->addWidget(defaultVolumeSpinBox);
    volumeLayout->addStretch();
    playbackLayout->addLayout(volumeLayout);

    playbackGroup->setLayout(playbackLayout);
    mainLayout->addWidget(playbackGroup);

    // Группа обработки повреждённых треков
    QGroupBox* badTracksGroup = new QGroupBox("⚠ Обработка повреждённых треков");
    QVBoxLayout* badTracksLayout = new QVBoxLayout;

    skipBadTracksCheckBox = new QCheckBox("Всегда пропускать повреждённые треки");
    skipBadTracksCheckBox->setToolTip("При встрече битого трека плеер автоматически перейдет к следующему");
    badTracksLayout->addWidget(skipBadTracksCheckBox);

    badTracksGroup->setLayout(badTracksLayout);
    mainLayout->addWidget(badTracksGroup);

    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout;

    cancelButton = new QPushButton("Отмена");
    cancelButton->setFixedWidth(100);
    buttonLayout->addWidget(cancelButton);

    buttonLayout->addStretch();

    saveButton = new QPushButton("Сохранить");
    saveButton->setFixedWidth(100);
    saveButton->setStyleSheet("QPushButton { font-weight: bold; background: #0078d4; color: white; }");
    buttonLayout->addWidget(saveButton);

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
