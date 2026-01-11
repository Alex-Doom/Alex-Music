#pragma once

#include <QMainWindow>      // Основное окно приложения
#include <QMediaPlayer>     // Медиаплеер Qt
#include <QAudioOutput>     // Аудиовыход
#include <QLabel>           // Текстовая метка
#include <QListWidget>      // Список элементов
#include <QLineEdit>        // Поле ввода текста
#include <QPushButton>      // Кнопка
#include "Playlist.h"       // Наш класс плейлиста
#include "PlayerControls.h" // Наш класс элементов управления

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
    void onTrackListDoubleClicked(QListWidgetItem* item); // Двойной клик по треку в списке
    void onMuteToggled(bool muted); // Включение/выключение звука
    void onRatingChanged(int rating); // Изменение рейтинга трека

    // Слоты для поиска и фильтрации
    void onSearchTextChanged(const QString& text); // Изменение текста поиска
    void onClearSearchClicked();    // Очистка поиска

    // Слоты для сортировки
    void onSortAlphabeticalClicked();  // Сортировка по алфавиту
    void onSortStandardClicked();      // Стандартная сортировка
    void onSortReverseClicked();       // Обратная сортировка

    void onScrollToCurrentClicked();   // Прокрутка к текущему треку

private:
    // Приватные методы
    void scanFolder(const QString& path);  // Сканирование папки с музыкой
    void playCurrentTrack();               // Воспроизведение текущего трека
    void updateUI();                       // Обновление интерфейса
    void restartCurrentTrack();            // Перезапуск текущего трека
    void setupRatingStars();               // Настройка звезд рейтинга

    // Методы для thumbnail toolbar (панель предпросмотра Windows)
    void setupThumbnailToolBar();     // Инициализация toolbar
    void updateThumbnailButtons();    // Обновление кнопок
    void cleanupThumbnailToolBar();   // Очистка ресурсов

    // Методы для сортировки и управления списком
    void applySorting(const std::vector<Track>& tracks, const QString& sortName);
    void updateSortButtonsStyle();    // Обновление стилей кнопок сортировки
    void highlightCurrentTrack();     // Подсветка текущего трека в списке

    // Основные объекты приложения
    Playlist playlist;                // Плейлист
    QMediaPlayer* player;             // Медиаплеер Qt
    QAudioOutput* audioOutput;        // Аудиовыход

    // Элементы интерфейса
    QLabel* coverLabel;               // Метка для обложки альбома
    QLabel* albumLabel;               // Метка названия альбома/трека
    QLabel* artistLabel;              // Метка имени исполнителя
    QLabel* genreLabel;               // Метка жанра (в данный момент не используется)
    QListWidget* trackList;           // Список треков
    PlayerControls* controls;         // Панель управления

    // Элементы поиска и фильтрации
    QLineEdit* searchEdit;            // Поле ввода для поиска
    QPushButton* clearSearchBtn;      // Кнопка очистки поиска

    QPushButton* scrollToCurrentBtn;  // Кнопка прокрутки к текущему треку

    // Кнопки сортировки
    QPushButton* sortAlphabeticalBtn; // Сортировка А-Я
    QPushButton* sortStandardBtn;     // Стандартная сортировка
    QPushButton* sortReverseBtn;      // Обратная сортировка

    // Данные для сортировки
    std::vector<Track> originalTracks_; // Оригинальный порядок треков
    bool isAlphabeticalSort_ = false;   // Флаг алфавитной сортировки
    bool isReverseSort_ = false;        // Флаг обратной сортировки

    // ЭЛЕМЕНТЫ ДЛЯ РЕЙТИНГА
    QPushButton* starButtons[5];      // Массив из 5 кнопок-звезд

    int volumeBeforeMute_ = 70;       // Громкость до отключения звука

    // Для thumbnail toolbar
    void* taskbarList = nullptr;      // Указатель на ITaskbarList3 (COM интерфейс)
    bool thumbnailToolbarInitialized = false; // Флаг инициализации

// Windows-specific методы и переменные
#ifdef Q_OS_WIN
    // Создание иконок из текста для thumbnail toolbar
    HICON createIconFromText(const wchar_t* text, int size = 16);
    HICON createPlayIcon();    // Создание иконки play
    HICON createPauseIcon();   // Создание иконки pause
    HICON createNextIcon();    // Создание иконки next
    HICON createPrevIcon();    // Создание иконки previous

    // Дескрипторы иконок Windows
    HICON playIcon = nullptr;
    HICON pauseIcon = nullptr;
    HICON nextIcon = nullptr;
    HICON prevIcon = nullptr;
#endif

    void setupShortcuts();  // Настройка горячих клавиш
};
