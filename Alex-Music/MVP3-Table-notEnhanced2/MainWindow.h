#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QLabel>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QProgressDialog>

#include "Playlist.h"
#include "PlayerControls.h"
#include "TrackValidator.h"
#include "SettingsDialog.h"
#include "BadTrackDialog.h"
#include "ScannerThread.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Для делегатов
    static int ratingColumn() { return COL_RATING; }

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

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

    // Таблица и навигация
    void onTableDoubleClicked(int row, int column);
    void onRatingChangedInTable(int row, int rating);
    void onHeaderClicked(int column);
    void playSelectedTrack();
    void onScrollToCurrentClicked();

    // Поиск и сортировка
    void onSearchTextChanged(const QString& text);
    void onSortAlphabeticalClicked();
    void onSortStandardClicked();
    void onSortReverseClicked();

    // Сканирование
    void scanFolder(const QString& path);
    void onTrackFound(const Track& track);
    void onScanProgress(int current, int total, const QString& currentFile);
    void onScanFinished(int totalTracks);

    // Меню и настройки
    void showSettingsDialog();
    void showHelpDialog();
    void showHotkeysDialog();
    void showAboutDialog();

    // Битые треки
    void handleInvalidTrack(const QString& filePath, const QString& error);

private:
    enum TableColumns {
        COL_TITLE = 0,
        COL_ARTIST,
        COL_GENRE,
        COL_ALBUM,
        COL_RATING,
        COL_YEAR,
        COL_COUNT
    };

    // Инициализация
    void setupUI();
    void setupTrackTable();
    void setupShortcuts();
    void createMenuBar();
    void setupThumbnailToolBar();

    // Воспроизведение
    void playCurrentTrack();
    void restartCurrentTrack();
    void updateUI();
    void updateTrackTableRow(int row);
    void highlightCurrentTrack();

    // Сортировка
    void applySorting(const std::vector<Track>& tracks, const QString& sortName);
    void updateSortButtonsStyle();

    // Навигация с пропуском битых треков
    bool navigateWithSkip(bool forward);
    bool navigateAutoSkip(bool forward);
    void showBadTrackDialog(const QString& filePath, bool wasForward);
    bool validateTrack(const QString& filePath);

    // Thumbnail toolbar (Windows)
    void updateThumbnailButtons();
    void cleanupThumbnailToolBar();

    // Настройки
    void saveSettings();
    void loadSettings();
    void updateMenuBar();

    // Поиск
    QString simpleHighlight(const QString& text, const QString& searchText) const;

    // Основные объекты
    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
    TrackValidator* trackValidator;
    ScannerThread* scannerThread;
    SettingsDialog* settingsDialog;

    // UI элементы
    QWidget* centralWidget;
    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QTableWidget* trackTable;
    PlayerControls* controls;
    QLineEdit* searchEdit;

    // Кнопки сортировки
    QPushButton* sortAlphabeticalBtn;
    QPushButton* sortStandardBtn;
    QPushButton* sortReverseBtn;
    QPushButton* scrollToCurrentBtn;

    // Меню
    QMenuBar* menuBar;
    QMenu* fileMenu;
    QMenu* settingsMenu;
    QMenu* helpMenu;

    // Рейтинг
    QPushButton* starButtons[5];

    // Данные
    std::vector<Track> originalTracks_;
    QStringList pendingFiles_;
    bool isAlphabeticalSort_ = false;
    bool isReverseSort_ = false;
    bool alwaysSkipBadTracks_ = false;
    bool lastWasForward_ = true;
    int volumeBeforeMute_ = 70;

    // Состояния
    bool savedShuffleState_ = false;
    Playlist::RepeatMode savedRepeatMode_ = Playlist::RepeatMode::None;

    // Windows specific
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
