#include "Playlist.h"
#include <random>
#include <fstream>
#include <sstream>
#include <QDir>
#include <QCoreApplication>
#include <algorithm>
#include <QDebug>

void Playlist::clear() {
    tracks_.clear();
    currentIndex_ = 0;
    corruptedTracks_.clear(); // Очищаем список битых треков

    while (!backStack_.empty()) backStack_.pop();
    while (!forwardStack_.empty()) forwardStack_.pop();

    shuffleQueue_.clear();
    currentQueuePosition_ = 0;
    maxPositivePosition_ = 0;
    minNegativePosition_ = 0;

    shuffle_ = false;
    repeatMode_ = RepeatMode::None;
}

std::optional<Track> Playlist::current() const {
    if (currentIndex_ >= tracks_.size()) return std::nullopt;
    return tracks_[currentIndex_];
}

bool Playlist::next() {
    if (tracks_.empty()) return false;

    // НОВОЕ: Пропускаем битые треки если включен авто-пропуск
    if (autoSkipCorrupted_) {
        int attempts = 0;
        while (attempts < static_cast<int>(tracks_.size())) {
            if (repeatMode_ == RepeatMode::One) {
                if (shuffle_) {
                    if (navigateInShuffleQueue(1)) {
                        if (!isTrackCorrupted(currentIndex_)) return true;
                    }
                } else {
                    size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
                    if (setCurrent(nextIdx)) {
                        if (!isTrackCorrupted(currentIndex_)) return true;
                    }
                }
            } else {
                if (shuffle_) {
                    if (navigateInShuffleQueue(1)) {
                        if (!isTrackCorrupted(currentIndex_)) return true;
                    }
                } else {
                    size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
                    if (setCurrent(nextIdx)) {
                        if (!isTrackCorrupted(currentIndex_)) return true;
                    }
                }
            }
            attempts++;
        }
        return false; // Все треки битые
    }

    if (repeatMode_ == RepeatMode::One) {
        if (shuffle_) {
            return navigateInShuffleQueue(1);
        } else {
            size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
            return setCurrent(nextIdx);
        }
    }

    if (shuffle_) {
        return navigateInShuffleQueue(1);
    }

    size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
    return setCurrent(nextIdx);
}

bool Playlist::prev(qint64 currentPosition) {
    if (tracks_.empty()) return false;

    if (shouldRestartTrack(currentPosition)) {
        return true;
    }

    // НОВОЕ: Пропускаем битые треки если включен авто-пропуск
    if (autoSkipCorrupted_) {
        int attempts = 0;
        while (attempts < static_cast<int>(tracks_.size())) {
            if (shuffle_) {
                if (navigateInShuffleQueue(-1)) {
                    if (!isTrackCorrupted(currentIndex_)) return true;
                }
            } else {
                if (standardPrevLogic(currentPosition)) {
                    if (!isTrackCorrupted(currentIndex_)) return true;
                }
            }
            attempts++;
        }
        return false; // Все треки битые
    }

    if (shuffle_) {
        return navigateInShuffleQueue(-1);
    }

    return standardPrevLogic(currentPosition);
}

bool Playlist::standardPrevLogic(qint64 currentPosition) {
    if (repeatMode_ == RepeatMode::One) {
        size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
        return setCurrent(prevIdx);
    }

    if (!backStack_.empty()) {
        forwardStack_.push(currentIndex_);
        currentIndex_ = backStack_.top();
        backStack_.pop();
        return true;
    }

    size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
    return setCurrent(prevIdx);
}

size_t Playlist::getRandomTrackIndex() const {
    if (tracks_.size() <= 1) return 0;

    std::uniform_int_distribution<size_t> dist(0, tracks_.size() - 1);
    size_t randomIndex;

    do {
        randomIndex = dist(rng_);
    } while (randomIndex == currentIndex_ && tracks_.size() > 1);

    return randomIndex;
}

size_t Playlist::getRandomTrackIndexExcluding(const std::vector<size_t>& excluded) const {
    if (tracks_.empty()) return 0;
    if (tracks_.size() <= excluded.size()) return 0;

    std::uniform_int_distribution<size_t> dist(0, tracks_.size() - 1);
    size_t randomIndex;

    do {
        randomIndex = dist(rng_);
        bool isExcluded = std::find(excluded.begin(), excluded.end(), randomIndex) != excluded.end();
        if (!isExcluded) {
            break;
        }
    } while (true);

    return randomIndex;
}

void Playlist::addToShuffleQueue(int queuePosition) {
    std::vector<size_t> usedTracks;
    for (const auto& pair : shuffleQueue_) {
        usedTracks.push_back(pair.second);
    }

    size_t randomTrackIndex = getRandomTrackIndexExcluding(usedTracks);
    shuffleQueue_[queuePosition] = randomTrackIndex;

    if (queuePosition > maxPositivePosition_) {
        maxPositivePosition_ = queuePosition;
    }
    if (queuePosition < minNegativePosition_) {
        minNegativePosition_ = queuePosition;
    }
}

bool Playlist::navigateInShuffleQueue(int direction) {
    int targetPosition = currentQueuePosition_ + direction;

    auto it = shuffleQueue_.find(targetPosition);
    if (it != shuffleQueue_.end()) {
        currentQueuePosition_ = targetPosition;
        currentIndex_ = it->second;
        return true;
    }

    addToShuffleQueue(targetPosition);
    currentQueuePosition_ = targetPosition;
    currentIndex_ = shuffleQueue_[targetPosition];

    return true;
}

void Playlist::setShuffle(bool enabled) {
    if (enabled && !shuffle_) {
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        maxPositivePosition_ = 0;
        minNegativePosition_ = 0;

        shuffleQueue_[0] = currentIndex_;
    } else if (!enabled && shuffle_) {
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        maxPositivePosition_ = 0;
        minNegativePosition_ = 0;
    }

    shuffle_ = enabled;
}

void Playlist::resetShuffleHistory() {
    shuffleQueue_.clear();
    currentQueuePosition_ = 0;
    maxPositivePosition_ = 0;
    minNegativePosition_ = 0;

    if (shuffle_) {
        shuffleQueue_[0] = currentIndex_;
    }
}

bool Playlist::shouldRestartTrack(qint64 currentPosition) const {
    return currentPosition > 3000;
}

bool Playlist::setCurrentTrackRating(double rating) {
    if (currentIndex_ >= tracks_.size()) return false;

    if (rating < 0.0 || rating > 5.0) return false;

    tracks_[currentIndex_].setTrackRating(rating);
    saveRatings();

    return true;
}

void Playlist::loadRatings() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt";

    std::ifstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string trackPath;
        double rating;

        if (std::getline(iss, trackPath, '|') && (iss >> rating)) {
            for (auto& track : tracks_) {
                if (track.path() == trackPath) {
                    track.setTrackRating(rating);
                    break;
                }
            }
        }
    }

    file.close();
}

void Playlist::saveRatings() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt";

    std::ofstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return;
    }

    for (const auto& track : tracks_) {
        if (track.rating() > 0.0) {
            file << track.path() << "|" << track.rating() << "\n";
        }
    }

    file.close();
}

bool Playlist::setCurrent(size_t i, bool resetShuffle) {
    if (i >= tracks_.size()) return false;

    if (currentIndex_ < tracks_.size() && shuffle_) {
        backStack_.push(currentIndex_);
    }

    while (!forwardStack_.empty()) forwardStack_.pop();

    currentIndex_ = i;

    if (resetShuffle) {
        setCurrentAsShuffleAnchor();
    }

    if (shuffle_) {
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        shuffleQueue_[0] = i;

        if (resetShuffle) {
            shuffleAnchorIndex_ = i;
        }
    }

    return true;
}

void Playlist::setCurrentAsShuffleAnchor() {
    if (tracks_.empty()) return;

    shuffleAnchorIndex_ = currentIndex_;

    if (shuffle_) {
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        shuffleQueue_[0] = currentIndex_;

        while (!backStack_.empty()) backStack_.pop();
        while (!forwardStack_.empty()) forwardStack_.pop();
    }
}

// НОВЫЕ МЕТОДЫ ДЛЯ РАБОТЫ С БИТЫМИ ТРЕКАМИ

bool Playlist::markTrackAsCorrupted(size_t index) {
    if (index >= tracks_.size()) return false;

    // Проверяем, не добавлен ли уже этот трек
    auto it = std::find(corruptedTracks_.begin(), corruptedTracks_.end(), index);
    if (it == corruptedTracks_.end()) {
        corruptedTracks_.push_back(index);
        // Сортируем для быстрого поиска
        std::sort(corruptedTracks_.begin(), corruptedTracks_.end());
        return true;
    }

    return false;
}

bool Playlist::isTrackCorrupted(size_t index) const {
    if (index >= tracks_.size()) return false;

    return std::binary_search(corruptedTracks_.begin(), corruptedTracks_.end(), index);
}

bool Playlist::skipCorruptedTrack(int direction) {
    if (tracks_.empty()) return false;

    size_t originalIndex = currentIndex_;
    size_t newIndex = currentIndex_;
    int attempts = 0;

    while (attempts < static_cast<int>(tracks_.size())) {
        if (direction > 0) {
            newIndex = (newIndex + 1) % tracks_.size();
        } else {
            newIndex = (newIndex == 0) ? tracks_.size() - 1 : newIndex - 1;
        }

        if (!isTrackCorrupted(newIndex)) {
            return setCurrent(newIndex);
        }

        attempts++;
    }

    // Все треки битые, возвращаемся на исходный
    setCurrent(originalIndex);
    return false;
}
