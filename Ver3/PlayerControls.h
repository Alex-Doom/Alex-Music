#pragma once
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QtGlobal>

class ClickableSlider;

class PlayerControls : public QWidget {
    Q_OBJECT
public:
    explicit PlayerControls(QWidget* parent = nullptr);

    void setPlaying(bool playing);
    void setPosition(qint64 position, qint64 duration);
    void setVolume(int volume);
    void setRepeatState(int state);
    void setShuffleState(bool shuffled);

    QString formatTime(qint64 milliseconds);

signals:
    void playPauseClicked();
    void nextClicked();
    void prevClicked();
    void seek(qint64 position);
    void volumeChanged(int volume);
    void repeatClicked();
    void shuffleClicked();
    void muteToggled(bool muted);

private slots:
    void onVolumeDownClicked();
    void onVolumeUpClicked();
    void onMuteClicked();

private:
    QPushButton* repeatBtn;
    QPushButton* shuffleBtn;
    QPushButton* prevBtn;
    QPushButton* playBtn;
    QPushButton* nextBtn;
    ClickableSlider* progressSlider;
    QSlider* volumeSlider;
    QLabel* timeLabel;

    QPushButton* volumeDownBtn;
    QPushButton* volumeUpBtn;
    QPushButton* muteBtn;

    int repeatState_ = 0;
    bool isShuffled_ = false;
    qint64 duration_ = 0;
    bool isMuted_ = false;
    int volumeBeforeMute_ = 70;
};

class ClickableSlider : public QSlider {
    Q_OBJECT
public:
    using QSlider::QSlider;

protected:
    void mousePressEvent(QMouseEvent* event) override;
};
