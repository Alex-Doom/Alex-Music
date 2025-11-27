#include "PlayerControls.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QStyle>

void ClickableSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int value = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                    event->position().x(), width());
        setValue(value);
        emit sliderReleased();
    }
    QSlider::mousePressEvent(event);
}

PlayerControls::PlayerControls(QWidget* parent) : QWidget(parent) {
    repeatBtn = new QPushButton("🔁");
    shuffleBtn = new QPushButton("🔀");
    prevBtn = new QPushButton("⏮");
    playBtn = new QPushButton("▶");
    playBtn->setStyleSheet("QPushButton { font-size: 24px; min-width: 60px; min-height: 60px; border-radius: 30px; }");
    nextBtn = new QPushButton("⏭");

    progressSlider = new ClickableSlider(Qt::Horizontal);
    progressSlider->setRange(0, 1000);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(70);
    volumeSlider->setFixedWidth(80);

    volumeDownBtn = new QPushButton("−");
    volumeDownBtn->setFixedSize(25, 25);
    volumeDownBtn->setStyleSheet("QPushButton { border-radius: 12px; font-weight: bold; background: #333; color: #fff; }");

    volumeUpBtn = new QPushButton("+");
    volumeUpBtn->setFixedSize(25, 25);
    volumeUpBtn->setStyleSheet("QPushButton { border-radius: 12px; font-weight: bold; background: #333; color: #fff; }");

    muteBtn = new QPushButton("🔊");
    muteBtn->setFixedSize(25, 25);
    muteBtn->setStyleSheet("QPushButton { border-radius: 12px; background: #333; color: #fff; }");

    timeLabel = new QLabel("00:00 / 00:00");
    timeLabel->setStyleSheet("QLabel { color: #fff; font-size: 12px; }");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 10);
    mainLayout->setSpacing(5);

    auto* progressLayout = new QHBoxLayout;
    progressLayout->addWidget(progressSlider, 1);
    progressLayout->addWidget(timeLabel);

    auto* controlsLayout = new QHBoxLayout;
    controlsLayout->setSpacing(15);

    controlsLayout->addWidget(repeatBtn);
    controlsLayout->addWidget(shuffleBtn);
    controlsLayout->addWidget(prevBtn);
    controlsLayout->addWidget(playBtn, 0, Qt::AlignCenter);
    controlsLayout->addWidget(nextBtn);
    controlsLayout->addStretch();
    controlsLayout->addWidget(volumeDownBtn);
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(volumeUpBtn);
    controlsLayout->addWidget(muteBtn);

    mainLayout->addLayout(progressLayout);
    mainLayout->addLayout(controlsLayout);

    progressSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    background: #333;"
        "    height: 6px;"
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #0078d4;"
        "    width: 16px;"
        "    height: 16px;"
        "    border-radius: 8px;"
        "    margin: -5px 0;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    background: #0086f0;"
        "    width: 18px;"
        "    height: 18px;"
        "}"
        );

    volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    background: #333;"
        "    height: 4px;"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #0078d4;"
        "    width: 12px;"
        "    height: 12px;"
        "    border-radius: 6px;"
        "    margin: -4px 0;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    background: #0086f0;"
        "}"
        );

    connect(playBtn, &QPushButton::clicked, this, &PlayerControls::playPauseClicked);
    connect(prevBtn, &QPushButton::clicked, this, &PlayerControls::prevClicked);
    connect(nextBtn, &QPushButton::clicked, this, &PlayerControls::nextClicked);
    connect(repeatBtn, &QPushButton::clicked, this, &PlayerControls::repeatClicked);
    connect(shuffleBtn, &QPushButton::clicked, this, &PlayerControls::shuffleClicked);

    connect(progressSlider, &QSlider::sliderReleased, this, [this]() {
        qint64 position = static_cast<qint64>(progressSlider->value() / 1000.0 * duration_);
        emit seek(position);
    });

    connect(volumeSlider, &QSlider::valueChanged, this, &PlayerControls::volumeChanged);

    connect(volumeDownBtn, &QPushButton::clicked, this, &PlayerControls::onVolumeDownClicked);
    connect(volumeUpBtn, &QPushButton::clicked, this, &PlayerControls::onVolumeUpClicked);
    connect(muteBtn, &QPushButton::clicked, this, &PlayerControls::onMuteClicked);
}

void PlayerControls::setPlaying(bool playing) {
    playBtn->setText(playing ? "⏸" : "▶");
}

void PlayerControls::setPosition(qint64 position, qint64 duration) {
    duration_ = duration;

    progressSlider->blockSignals(true);
    if (duration > 0) {
        int value = static_cast<int>((position * 1000.0) / duration);
        progressSlider->setValue(value);
    } else {
        progressSlider->setValue(0);
    }
    progressSlider->blockSignals(false);

    timeLabel->setText(formatTime(position) + " / " + formatTime(duration));
}

void PlayerControls::setVolume(int volume) {
    volumeSlider->blockSignals(true);
    volumeSlider->setValue(volume);
    volumeSlider->blockSignals(false);
}

void PlayerControls::setRepeatState(int state) {
    repeatState_ = state;
    switch (state) {
    case 0: repeatBtn->setText("🔁"); break;
    case 1: repeatBtn->setText("🔂"); break;
    case 2: repeatBtn->setText("🔁"); break;
    }
    QString activeStyle = "QPushButton { background: #0078d4; color: #fff; }";
    QString normalStyle = "QPushButton { background: #333; color: #fff; }";
    repeatBtn->setStyleSheet(state > 0 ? activeStyle : normalStyle);
}

void PlayerControls::setShuffleState(bool shuffled) {
    isShuffled_ = shuffled;
    QString activeStyle = "QPushButton { background: #0078d4; color: #fff; }";
    QString normalStyle = "QPushButton { background: #333; color: #fff; }";
    shuffleBtn->setStyleSheet(shuffled ? activeStyle : normalStyle);
}

QString PlayerControls::formatTime(qint64 milliseconds) {
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void PlayerControls::onVolumeUpClicked() {
    int newVolume = volumeSlider->value() + 10;
    if (newVolume > 100) newVolume = 100;
    volumeSlider->setValue(newVolume);
    emit volumeChanged(newVolume);
}

void PlayerControls::onVolumeDownClicked() {
    int newVolume = volumeSlider->value() - 10;
    if (newVolume < 0) newVolume = 0;
    volumeSlider->setValue(newVolume);
    emit volumeChanged(newVolume);
}

void PlayerControls::toggleMute() {
    isMuted_ = !isMuted_;
    if (isMuted_) {
        volumeBeforeMute_ = volumeSlider->value();
        muteBtn->setText("🔇");
        emit muteToggled(true);
    } else {
        muteBtn->setText("🔊");
        emit muteToggled(false);
    }
}

void PlayerControls::onMuteClicked() {
    isMuted_ = !isMuted_;
    if (isMuted_) {
        volumeBeforeMute_ = volumeSlider->value();
        muteBtn->setText("🔇");
        emit muteToggled(true);
    } else {
        muteBtn->setText("🔊");
        emit muteToggled(false);
    }
}
