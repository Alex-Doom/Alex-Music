#pragma once
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class ClickableSlider : public QSlider {
    Q_OBJECT
public:
    using QSlider::QSlider;
protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            int val = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                      event->position().x(), width());
            setValue(val);
            emit sliderReleased();
        }
        QSlider::mousePressEvent(event);
    }
};

class PlayerControls : public QWidget {
    Q_OBJECT
public:
    explicit PlayerControls(QWidget* parent = nullptr);

    void setPlaying(bool playing);
    void setPosition(qint64 pos, qint64 duration);
    void setVolume(int vol);
    void setShuffle(bool on);
    void setRepeat(bool on);

    int volume() const { return volumeSlider_->value(); }

signals:
    void playPause();
    void next();
    void prev();
    void seek(qint64 position);
    void volumeChanged(int vol);
    void shuffleToggled();
    void repeatToggled();
    void muteToggled(bool muted);

private:
    QPushButton* playBtn_;
    QPushButton* prevBtn_;
    QPushButton* nextBtn_;
    QPushButton* shuffleBtn_;
    QPushButton* repeatBtn_;
    QPushButton* muteBtn_;
    ClickableSlider* progressSlider_;
    QSlider* volumeSlider_;
    QLabel* timeLabel_;
    bool isMuted_ = false;
    int lastVolume_ = 70;

    QString formatTime(qint64 ms);
};
