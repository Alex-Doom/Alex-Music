#pragma once
#include "Track.h"
#include <vector>   // Контейнер вектор для хранения треков
#include <stack>    // Стек для истории навигации
#include <optional> // Для optional значений (может содержать значение или быть пустым)
#include <random>   // Для генерации случайных чисел
#include <QtGlobal> // Основные определения Qt
#include <map>      // Ассоциативный массив для shuffle очереди (для режима случайного порядка треков)

// управляет списком воспроизведения
class Playlist {
public:
    // режимы повтора треков
    enum class RepeatMode { None, One, /*All */};

    // Добавление трека в плейлист
    void add(const Track& t) { tracks_.push_back(t); }
    void clear(); // Очищает плейлист

    // текущий трек или nullopt - если плейлист пуст
    std::optional<Track> current() const;

    bool next(); // Переход к следующему треку
    // Удаляем старый метод или делаем его private
    // bool prev(qint64 currentPosition = 0); // УДАЛИТЬ ЭТУ СТРОКУ

    // Добавляем новый метод с дополнительным параметром
    bool prev(qint64 currentPosition = 0, bool skipThreeSecondRule = false);

    // Проверка возможности перехода назад/вперед
    bool canGoBack() const { return !backStack_.empty(); }
    bool canGoForward() const { return !forwardStack_.empty(); }

    // Возвращают: все треки плейлиста
    const std::vector<Track>& all() const { return tracks_; }
    // индекс текущего трека
    size_t currentIndex() const { return currentIndex_; }
    // количество треков в плейлисте
    size_t size() const { return tracks_.size(); }

    // Включает/выключает случайное воспроизведение
    void setShuffle(bool enabled);
    // Проверяет включен ли shuffle режим
    bool isShuffled() const { return shuffle_; }

    // Устанавливает режим повтора
    void setRepeatMode(RepeatMode mode) { repeatMode_ = mode; }
    // Возвращает текущий режим повтора
    RepeatMode repeatMode() const { return repeatMode_; }

    size_t getRandomTrackIndex() const; // Генерация индекс случайного трека
    void resetShuffleHistory();// Сбрасывает историю shuffle

    // Проверяет нужно ли перезапускать текущий трек (правило 3 секунд)
    bool shouldRestartTrack(qint64 currentPosition) const;

    bool setCurrentTrackRating(double rating); // рейтинг текущего трека

    void loadRatings(); // Загружает рейтинги из файла
    void saveRatings(); // Сохраняет рейтинги в файл

    // Устанавливает текущий трек (трек отсчета) как якорь для shuffle
    void setCurrentAsShuffleAnchor();
    // Устанавливает текущий трек по индексу
    bool setCurrent(size_t i, bool resetShuffle = false);

    void setSkipInvalidTracks(bool skip);

    // Проверка возможности перехода в направлении с учетом битых треков
    bool canNavigate(bool forward) const;

private:
    std::vector<Track> tracks_;       // Вектор всех треков
    size_t currentIndex_ = 0;         // Индекс текущего трека
    std::stack<size_t> backStack_;    // Стек истории назад
    std::stack<size_t> forwardStack_; // Стек истории вперед

    bool shuffle_ = false;                    // Флаг shuffle режима
    RepeatMode repeatMode_ = RepeatMode::None; // Текущий режим повтора трека
    mutable std::mt19937 rng_{std::random_device{}()}; // Генератор случайных чисел

    // Структура данных для shuffle режима
    // Ключ - позиция в очереди, значение - индекс трека
    std::map<int, size_t> shuffleQueue_;
    int currentQueuePosition_ = 0;    // Текущая позиция в shuffle очереди
    int maxPositivePosition_ = 0;     // Максимальная положительная позиция
    int minNegativePosition_ = 0;     // Минимальная отрицательная позиция

    // Хэш-таблица для сохранения рейтингов: путь к файлу -> рейтинг
    std::unordered_map<std::string, double> savedRatings_;

    // Вспомогательные методы
    // Генерирует случайный индекс исключая указанные треки
    size_t getRandomTrackIndexExcluding(const std::vector<size_t>& excluded) const;
    // Добавляет трек в shuffle очередь
    void addToShuffleQueue(int queuePosition);
    // Навигация в shuffle очереди
    bool navigateInShuffleQueue(int direction);

    // метод для стандартной логики навигации
    bool standardPrevLogic(qint64 currentPosition);

    size_t shuffleAnchorIndex_ = 0;  // Индекс якоря для shuffle


    // Флаг для пропуска битых треков
    bool skipInvalidTracks_ = false;

    // Вспомогательный метод для безопасного перехода к следующему треку
    bool safeNavigate(bool forward, int maxAttempts = 100);

    // методы для навигации без рекурсии
    bool nextInternal();  // Внутренний метод без вызова safeNavigate
    bool prevInternal(qint64 currentPosition = 0);  // Внутренний метод
};
