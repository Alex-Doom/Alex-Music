#include "Dialogs.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Настройки");
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("<b>Настройки воспроизведения</b>"));

    skipCheck_ = new QCheckBox("Автоматически пропускать повреждённые треки");
    layout->addWidget(skipCheck_);

    auto* volLayout = new QHBoxLayout;
    volLayout->addWidget(new QLabel("Громкость по умолчанию:"));
    volumeSpin_ = new QSpinBox;
    volumeSpin_->setRange(0, 100);
    volumeSpin_->setSuffix("%");
    volLayout->addWidget(volumeSpin_);
    volLayout->addStretch();
    layout->addLayout(volLayout);

    auto* buttons = new QHBoxLayout;
    auto* ok = new QPushButton("Сохранить");
    auto* cancel = new QPushButton("Отмена");
    buttons->addStretch();
    buttons->addWidget(cancel);
    buttons->addWidget(ok);
    layout->addLayout(buttons);

    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
}

bool SettingsDialog::autoSkipBadTracks() const { return skipCheck_->isChecked(); }
int SettingsDialog::defaultVolume() const { return volumeSpin_->value(); }
void SettingsDialog::setAutoSkip(bool skip) { skipCheck_->setChecked(skip); }
void SettingsDialog::setVolume(int vol) { volumeSpin_->setValue(vol); }

BadTrackDialog::BadTrackDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Ошибка воспроизведения");
    setModal(true);
    auto* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("⚠️ <b>Не удалось воспроизвести трек</b>"));
    layout->addWidget(new QLabel("Файл может быть повреждён или недоступен."));

    alwaysSkip_ = new QCheckBox("Больше не показывать (всегда пропускать)");
    layout->addWidget(alwaysSkip_);

    auto* buttons = new QHBoxLayout;
    auto* skip = new QPushButton("Пропустить");
    auto* stop = new QPushButton("Остановить");
    buttons->addWidget(stop);
    buttons->addWidget(skip);
    layout->addLayout(buttons);

    connect(skip, &QPushButton::clicked, this, &QDialog::accept);
    connect(stop, &QPushButton::clicked, this, &QDialog::reject);
}

void BadTrackDialog::setTrack(const QString& path, const QString& error) {
    QFileInfo info(path);
    setWindowTitle("Ошибка: " + info.fileName());
}

bool BadTrackDialog::skipAlways() const { return alwaysSkip_->isChecked(); }
