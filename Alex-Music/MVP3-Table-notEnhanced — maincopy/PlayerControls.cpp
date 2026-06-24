#include "PlayerControls.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>

PlayerControls::PlayerControls(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);

    // Прогресс и время
    auto* progressLayout = new QHBoxLayout;
    progressSlider_ = new ClickableSlider(Qt::Horizontal);
    progressSlider_->setRange(0, 1000);
    timeLabel_ = new QLabel("00:00 / 00:00");
    timeLabel_->setStyleSheet("font-family: monospace;");

    progressLayout->addWidget(progressSlider_, 1);
    progressLayout->addWidget(timeLabel_);
    layout->addLayout(progressLayout);

    // Кнопки управления
    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(10);

    repeatBtn_ = new QPushButton("🔁");
    repeatBtn_->setToolTip("Повтор");
    shuffleBtn_ = new QPushButton("🔀");
    shuffleBtn_->setToolTip("Случайный порядок");
    prevBtn_ = new QPushButton("⏮");
    playBtn_ = new QPushButton("▶");
    playBtn_->setFixedSize(50, 50);
    playBtn_->setStyleSheet("font-size: 20px; border-radius: 25px;");
    nextBtn_ = new QPushButton("⏭");

    volumeSlider_ = new QSlider(Qt::Horizontal);
    volumeSlider_->setRange(0, 100);
    volumeSlider_->setValue(70);
    volumeSlider_->setFixedWidth(100);
    muteBtn_ = new QPushButton("🔊");

    btnLayout->addWidget(repeatBtn_);
    btnLayout->addWidget(shuffleBtn_);
    btnLayout->addWidget(prevBtn_);
    btnLayout->addWidget(playBtn_);
    btnLayout->addWidget(nextBtn_);
    btnLayout->addStretch();
    btnLayout->addWidget(muteBtn_);
    btnLayout->addWidget(volumeSlider_);

    layout->addLayout(btnLayout);

    // Стили
    QString btnStyle = "QPushButton { background: #333; color: white; border-radius: 4px; padding: 5px; }"
                       "QPushButton:hover { background: #444; }";
    for (auto* btn : {repeatBtn_, shuffleBtn_, prevBtn_, nextBtn_, muteBtn_}) {
        btn->setStyleSheet(btnStyle);
    }

    // Соединения
    connect(playBtn_, &QPushButton::clicked, this, &PlayerControls::playPause);
    connect(prevBtn_, &QPushButton::clicked, this, &PlayerControls::prev);
    connect(nextBtn_, &QPushButton::clicked, this, &PlayerControls::next);
    connect(repeatBtn_, &QPushButton::clicked, this, &PlayerControls::repeatToggled);
    connect(shuffleBtn_, &QPushButton::clicked, this, &PlayerControls::shuffleToggled);
    connect(volumeSlider_, &QSlider::valueChanged, this, &PlayerControls::volumeChanged);
    connect(progressSlider_, &QSlider::sliderReleased, [this]() {
        emit seek(progressSlider_->value());
    });
    connect(muteBtn_, &QPushButton::clicked, [this]() {
        isMuted_ = !isMuted_;
        muteBtn_->setText(isMuted_ ? "🔇" : "🔊");
        emit muteToggled(isMuted_);
    });
}

void PlayerControls::setPlaying(bool playing) {
    playBtn_->setText(playing ? "⏸" : "▶");
}

void PlayerControls::setPosition(qint64 pos, qint64 duration) {
    if (duration > 0) {
        progressSlider_->blockSignals(true);
        progressSlider_->setValue(static_cast<int>((pos * 1000) / duration));
        progressSlider_->blockSignals(false);
        timeLabel_->setText(formatTime(pos) + " / " + formatTime(duration));
    }
}

void PlayerControls::setVolume(int vol) {
    volumeSlider_->setValue(vol);
}

void PlayerControls::setShuffle(bool on) {
    shuffleBtn_->setStyleSheet(on ?
                                   "background: #0078d4; color: white; border-radius: 4px; padding: 5px;" :
                                   "background: #333; color: white; border-radius: 4px; padding: 5px;");
}

void PlayerControls::setRepeat(bool on) {
    repeatBtn_->setStyleSheet(on ?
                                  "background: #0078d4; color: white; border-radius: 4px; padding: 5px;" :
                                  "background: #333; color: white; border-radius: 4px; padding: 5px;");
}

QString PlayerControls::formatTime(qint64 ms) {
    qint64 sec = ms / 1000;
    qint64 min = sec / 60;
    sec = sec % 60;
    return QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
}
