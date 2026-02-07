#pragma once
#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QListWidget>
#include "Playlist.h"
#include "PlayerControls.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onPlayPauseClicked();
    void onNextClicked();
    void onPrevClicked();
    void onRepeatClicked();
    void onShuffleClicked();
    void onSeek(qint64 position);
    void onVolumeChanged(int volume);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onTrackListDoubleClicked(QListWidgetItem* item);
    void onMuteToggled(bool muted);
    void onRatingChanged(int rating);  // НОВЫЙ СЛОТ для рейтинга

private:
    void scanFolder(const QString& path);
    void playCurrentTrack();
    void updateUI();
    void restartCurrentTrack();
    void setupRatingStars();  // НОВЫЙ МЕТОД для настройки звезд

    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QLabel* genreLabel;
    QListWidget* trackList;
    PlayerControls* controls;

    // НОВЫЕ ЭЛЕМЕНТЫ ДЛЯ РЕЙТИНГА
    QPushButton* starButtons[5];  // 5 кнопок-звезд

    int volumeBeforeMute_ = 70;
};
