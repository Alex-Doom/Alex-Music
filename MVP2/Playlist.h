#pragma once
#include "Track.h"
#include <vector>
#include <stack>
#include <optional>
#include <random>
#include <QtGlobal>
#include <map>
#include <unordered_map>
#include <string>

class Playlist {
public:
    enum class RepeatMode { None, One };

    void add(const Track& t) { tracks_.push_back(t); }
    void clear();

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

    void setCurrentAsShuffleAnchor();
    bool setCurrent(size_t i, bool resetShuffle = false);

    // НОВОЕ: Методы для работы с битыми треками
    bool markTrackAsCorrupted(size_t index);
    bool isTrackCorrupted(size_t index) const;
    bool skipCorruptedTrack(int direction = 1); // 1 - вперед, -1 - назад
    const std::vector<size_t>& getCorruptedTracks() const { return corruptedTracks_; }
    void setAutoSkipCorrupted(bool skip) { autoSkipCorrupted_ = skip; }
    bool getAutoSkipCorrupted() const { return autoSkipCorrupted_; }

private:
    std::vector<Track> tracks_;
    size_t currentIndex_ = 0;
    std::stack<size_t> backStack_;
    std::stack<size_t> forwardStack_;

    bool shuffle_ = false;
    RepeatMode repeatMode_ = RepeatMode::None;
    mutable std::mt19937 rng_{std::random_device{}()};

    std::map<int, size_t> shuffleQueue_;
    int currentQueuePosition_ = 0;
    int maxPositivePosition_ = 0;
    int minNegativePosition_ = 0;

    std::unordered_map<std::string, double> savedRatings_;

    // НОВОЕ: Хранение индексов битых треков
    std::vector<size_t> corruptedTracks_;
    bool autoSkipCorrupted_ = false;

    size_t getRandomTrackIndexExcluding(const std::vector<size_t>& excluded) const;
    void addToShuffleQueue(int queuePosition);
    bool navigateInShuffleQueue(int direction);
    bool standardPrevLogic(qint64 currentPosition);

    size_t shuffleAnchorIndex_ = 0;
};
