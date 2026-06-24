// MainWindow.h
#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

#include "Playlist.h"
#include "PlayerControls.h"
#include "TrackValidator.h"
#include "SettingsDialog.h"
#include "BadTrackDialog.h"
#include "Mp3Metadata.h"
#include "FastMetadataReader.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static int ratingColumn() { return COL_RATING; }

    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    // Управление воспроизведением
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
    void onMuteToggled(bool muted);
    void onRatingChanged(int rating);

    // Поиск и сортировка
    void onSearchTextChanged(const QString& text);
    QString simpleHighlight(const QString& text, const QString& searchText) const;
    void onSortAlphabeticalClicked();
    void onSortStandardClicked();
    void onSortReverseClicked();
    void onScrollToCurrentClicked();
    void onHeaderClicked(int column);

    // Управление треками
    void playSelectedTrack();
    void showSettingsDialog();
    void onTableDoubleClicked(int row, int column);
    void onRatingChangedInTable(int row, int rating);
    void onMetadataLoaded(const QString& filePath, const TrackMetadata& metadata);

private:
    // Инициализация
    void setupTrackTable();
    void setupShortcuts();
    void createMenuBar();
    void updateMenuBar();

    // Управление плейлистом
    void scanFolder(const QString& path);
    void playCurrentTrack();
    void updateUI();
    void restartCurrentTrack();
    void populateTrackTable();
    void updateTrackTableRow(int row);
    void highlightCurrentTrack();

    // Сортировка
    void applySorting(const std::vector<Track>& tracks, const QString& sortName);
    void updateSortButtonsStyle();

    // Метаданные
    void loadRealMetadataInBackground(const QStringList& filePaths);

    // Навигация
    bool navigateWithSkip(bool forward);
    bool navigateAutoSkip(bool forward);
    bool navigateWithDialog(bool forward);
    void showBadTrackDialog(const QString& filePath, bool wasForward);
    bool hasValidTrackInDirection(bool forward, int maxAttempts = 100);

    // Валидация
    bool validateTrack(const QString& filePath);
    void handleInvalidTrack(const QString& filePath, const QString& error);

    // Настройки
    void saveSettings();
    void loadSettings();

    // Windows thumbnail toolbar
    void setupThumbnailToolBar();
    void updateThumbnailButtons();
    void cleanupThumbnailToolBar();

    // Диалоги
    void showHelpDialog();
    void showHotkeysDialog();
    void showAboutDialog();
    void setupRatingStars();

    enum TableColumns {
        COL_TITLE = 0,
        COL_ARTIST,
        COL_GENRE,
        COL_ALBUM,
        COL_RATING,
        COL_YEAR,
        COL_COUNT
    };

    // Основные объекты
    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
    TrackValidator* trackValidator;
    SettingsDialog* settingsDialog;

    // UI элементы
    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QTableWidget* trackTable;
    PlayerControls* controls;
    QLineEdit* searchEdit;
    QPushButton* sortAlphabeticalBtn;
    QPushButton* sortStandardBtn;
    QPushButton* sortReverseBtn;
    QPushButton* starButtons[5];

    // Меню
    QMenuBar* menuBar;
    QMenu* fileMenu;
    QMenu* settingsMenu;
    QMenu* helpMenu;

    // Данные
    std::vector<Track> originalTracks_;
    bool isAlphabeticalSort_ = false;
    bool isReverseSort_ = false;
    bool alwaysSkipBadTracks_ = false;
    bool lastWasForward_ = true;
    int volumeBeforeMute_ = 70;

    // Состояния
    bool savedShuffleState_ = false;
    Playlist::RepeatMode savedRepeatMode_ = Playlist::RepeatMode::None;

    // Windows thumbnail toolbar
    void* taskbarList = nullptr;
    bool thumbnailToolbarInitialized = false;

#ifdef Q_OS_WIN
    HICON createIconFromText(const wchar_t* text, int size = 16);
    HICON createPlayIcon();
    HICON createPauseIcon();
    HICON createNextIcon();
    HICON createPrevIcon();
    HICON playIcon = nullptr;
    HICON pauseIcon = nullptr;
    HICON nextIcon = nullptr;
    HICON prevIcon = nullptr;
#endif
};
