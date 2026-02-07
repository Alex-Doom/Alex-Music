#pragma once
#include "Track.h"
#include <vector>
#include <stack>
#include <optional>
#include <random>
// #include <algorithm>
#include <QtGlobal>
#include <map>

class Playlist {
public:
    enum class RepeatMode { None, One, All };

    void add(const Track& t) { tracks_.push_back(t); }
    void clear();

    bool setCurrent(size_t i);
    std::optional<Track> current() const;

    bool next();
    bool prev(qint64 currentPosition = 0);
    bool canGoBack() const { return !backStack_.empty(); }
    bool canGoForward() const { return !forwardStack_.empty(); }

    const std::vector<Track>& all() const { return tracks_; }
    size_t currentIndex() const { return currentIndex_; }
    size_t size() const { return tracks_.size(); }

    void setShuffle(bool enabled);
    bool isShuffled() const { return shuffle_; }
    void setRepeatMode(RepeatMode mode) { repeatMode_ = mode; }
    RepeatMode repeatMode() const { return repeatMode_; }

    size_t getRandomTrackIndex() const;
    void resetShuffleHistory();
    bool shouldRestartTrack(qint64 currentPosition) const;

     bool setCurrentTrackRating(double rating);

    void loadRatings();
    void saveRatings();
    // void applySavedRatings();

private:
    std::vector<Track> tracks_;
    size_t currentIndex_ = 0;
    std::stack<size_t> backStack_;
    std::stack<size_t> forwardStack_;

    bool shuffle_ = false;
    RepeatMode repeatMode_ = RepeatMode::None;
    mutable std::mt19937 rng_{std::random_device{}()};

    // Структура данных для shuffle режима
    std::map<int, size_t> shuffleQueue_;
    int currentQueuePosition_ = 0;
    int maxPositivePosition_ = 0;
    int minNegativePosition_ = 0;

    std::unordered_map<std::string, double> savedRatings_;

    // Вспомогательные методы
    size_t getRandomTrackIndexExcluding(const std::vector<size_t>& excluded) const;
    void addToShuffleQueue(int queuePosition);
    bool navigateInShuffleQueue(int direction);

    // РОДИТЕЛЬСКИЙ МЕТОД для стандартной логики навигации
    bool standardPrevLogic(qint64 currentPosition);
};
