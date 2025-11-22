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
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void showEvent(QShowEvent* event) override;

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
    void onRatingChanged(int rating);

private:
    void scanFolder(const QString& path);
    void playCurrentTrack();
    void updateUI();
    void restartCurrentTrack();
    void setupRatingStars();  // НОВЫЙ МЕТОД для настройки звезд

    // Методы для thumbnail toolbar
    void setupThumbnailToolBar();
    void updateThumbnailButtons();
    void cleanupThumbnailToolBar();

    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QLabel* genreLabel;
    QListWidget* trackList;
    PlayerControls* controls;

    // ЭЛЕМЕНТЫ ДЛЯ РЕЙТИНГА
    QPushButton* starButtons[5];  // 5 кнопок-звезд

    int volumeBeforeMute_ = 70;

    // Для thumbnail toolbar
    void* taskbarList = nullptr;  // ITaskbarList3 pointer
    bool thumbnailToolbarInitialized = false;

#ifdef Q_OS_WIN
    HICON createIconFromText(const wchar_t* text, int size = 16);
    HICON createPlayIcon();
    HICON createPauseIcon();
    HICON createNextIcon();
    HICON createPrevIcon();
#endif

#ifdef Q_OS_WIN
    HICON playIcon = nullptr;
    HICON pauseIcon = nullptr;
    HICON nextIcon = nullptr;
    HICON prevIcon = nullptr;
#endif
};
