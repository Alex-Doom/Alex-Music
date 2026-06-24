#pragma once

#include <QMainWindow>      // Основное окно приложения
#include <QMediaPlayer>     // Медиаплеер Qt
#include <QAudioOutput>     // Аудиовыход
#include <QLabel>           // Текстовая метка
#include <QTableWidget>  // ЗАМЕНЯЕМ QListWidget на QTableWidget - для столбиков в списке треков
#include <QHeaderView> // Для управления заголовками таблицы
#include <QLineEdit>        // Поле ввода текста
#include <QPushButton>      // Кнопка
#include <QSettings>

#include "Playlist.h"       // Наш класс плейлиста
#include "PlayerControls.h" // Наш класс элементов управления
#include "TrackValidator.h"
#include "SettingsDialog.h"
#include "BadTrackDialog.h"
#include "RatingDelegate.h"


// Главное окно приложения
class MainWindow : public QMainWindow {
    Q_OBJECT  // Макрос для поддержки сигналов и слотов

public:
    // Конструктор и деструктор
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Переопределенные методы для обработки системных событий
protected:
    // Обработчик нативных событий Windows (для thumbnail toolbar)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    // Обработчик события показа окна
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

// Слоты - методы для обработки сигналов
private slots:
    // Слоты для элементов управления
    void onPlayPauseClicked();      // Обработка play/pause
    void onNextClicked();           // Следующий трек
    void onPrevClicked();           // Предыдущий трек
    void onRepeatClicked();         // Переключение повтора
    void onShuffleClicked();        // Переключение перемешивания
    void onSeek(qint64 position);   // Перемотка трека
    void onVolumeChanged(int volume); // Изменение громкости
    void onPositionChanged(qint64 position); // Изменение позиции трека
    void onDurationChanged(qint64 duration); // Изменение длительности трека
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status); // Изменение статуса медиа
    void onTrackTableDoubleClicked(int row, int column); // Двойной клик по треку в списке
    void onMuteToggled(bool muted); // Включение/выключение звука
    void onRatingChanged(int rating); // Изменение рейтинга трека

    // Слоты для поиска и фильтрации
    void onSearchTextChanged(const QString& text); // Изменение текста поиска
    QString simpleHighlight(const QString& text, const QString& searchText) const;

    // Слоты для сортировки
    void onTableHeaderClicked(int column);
    void onSortStandardClicked();      // Стандартная сортировка

    void onScrollToCurrentClicked();   // Прокрутка к текущему треку

    void playSelectedTrack();
    void showSettingsDialog();

private:
    void scanFolder(const QString& path);
    void playCurrentTrack();
    void updateUI();
    void restartCurrentTrack();
    void setupRatingStars();

    void setupThumbnailToolBar();
    void updateThumbnailButtons();
    void cleanupThumbnailToolBar();

    void highlightCurrentTrackInTable();
    void populateTrackTable();
    void setupTrackTable();
    void updateTrackTable();
    void applyTableSorting(int column, Qt::SortOrder order);
    void playTrackFromTable(int row);

    void saveSettings();
    void loadSettings();

    bool validateTrack(const QString& filePath);
    void handleInvalidTrack(const QString& filePath, const QString& error);
    void showBadTrackDialog(const QString& filePath, bool wasForward);

    bool navigateWithSkip(bool forward);
    bool navigateAutoSkip(bool forward);
    bool navigateWithDialog(bool forward);

    void setupShortcuts();

    Playlist playlist;
    QMediaPlayer* player;
    QAudioOutput* audioOutput;

    QPushButton* settingsBtn;
    SettingsDialog* settingsDialog;

    QLabel* coverLabel;
    QLabel* albumLabel;
    QLabel* artistLabel;
    QLabel* genreLabel;
    QTableWidget* trackTable;
    PlayerControls* controls;

    QLineEdit* searchEdit;
    QPushButton* clearSearchBtn;
    QPushButton* scrollToCurrentBtn;
    QPushButton* sortStandardBtn;

    std::vector<Track> originalTracks_;

    QPushButton* starButtons[5];
    int volumeBeforeMute_ = 70;

    void* taskbarList = nullptr;
    bool thumbnailToolbarInitialized = false;

    bool savedShuffleState_ = false;
    Playlist::RepeatMode savedRepeatMode_ = Playlist::RepeatMode::None;

    TrackValidator* trackValidator;
    bool alwaysSkipBadTracks_ = false;
    bool lastWasForward_ = true;

    int currentSortColumn_ = 1;
    Qt::SortOrder currentSortOrder_ = Qt::AscendingOrder;
    std::vector<size_t> originalOrder_;

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
