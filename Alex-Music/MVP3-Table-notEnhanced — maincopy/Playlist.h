#pragma once
#include "Track.h"
#include <vector>
#include <optional>
#include <random>
#include <QObject>

class Playlist : public QObject {
    Q_OBJECT
public:
    enum class RepeatMode { None, One };
    enum class SortMode { Default, Alphabetical, Reverse };

    explicit Playlist(QObject* parent = nullptr);

    // Управление треками
    void add(const Track& track);
    void clear();
    void reserve(size_t size) { tracks_.reserve(size); }
    size_t size() const { return tracks_.size(); }
    bool empty() const { return tracks_.empty(); }

    // Доступ
    std::optional<Track> current() const;
    const std::vector<Track>& tracks() const { return tracks_; }
    size_t currentIndex() const { return currentIndex_; }

    // Навигация
    bool next();
    bool prev(qint64 currentPositionMs = 0);
    bool setCurrent(size_t index);

    // Режимы
    void setShuffle(bool enabled);
    bool isShuffled() const { return shuffle_; }
    void setRepeatMode(RepeatMode mode) { repeatMode_ = mode; }
    RepeatMode repeatMode() const { return repeatMode_; }

    // Рейтинги
    void setRating(double rating);
    void loadRatings();
    void saveRatings();

    // Сортировка (визуальная, не меняет порядок воспроизведения при shuffle)
    void applySort(SortMode mode);
    SortMode sortMode() const { return sortMode_; }

signals:
    void currentChanged(size_t index);
    void ratingChanged(size_t index, double rating);

private:
    std::vector<Track> tracks_;
    size_t currentIndex_ = 0;
    std::vector<size_t> shuffleOrder_; // Индексы для shuffle
    size_t shufflePos_ = 0;

    bool shuffle_ = false;
    RepeatMode repeatMode_ = RepeatMode::None;
    SortMode sortMode_ = SortMode::Default;

    std::mt19937 rng_{std::random_device{}()};

    void generateShuffleOrder();
    void ensureShuffleOrder();
};
