#pragma once
#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
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

    void onSearchTextChanged(const QString& text);
    void onClearSearchClicked();

    void onSortAlphabeticalClicked();
    void onSortStandardClicked();
    void onSortReverseClicked();

    void onScrollToCurrentClicked();

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

    void applySorting(const std::vector<Track>& tracks, const QString& sortName);
    void updateSortButtonsStyle();
    void highlightCurrentTrack();

    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QLabel* genreLabel;
    QListWidget* trackList;
    PlayerControls* controls;

    QLineEdit* searchEdit;  // Строка поиска
    QPushButton* clearSearchBtn; // Кнопка очистки поиска

    QPushButton* scrollToCurrentBtn;

    QPushButton* sortAlphabeticalBtn;
    QPushButton* sortStandardBtn;
    QPushButton* sortReverseBtn;

    std::vector<Track> originalTracks_;
    bool isAlphabeticalSort_ = false;
    bool isReverseSort_ = false;

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

    void setupShortcuts();
};
