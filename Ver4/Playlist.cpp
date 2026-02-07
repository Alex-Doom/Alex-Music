#include "Playlist.h"
#include <random>
#include <fstream>
#include <sstream>
#include <QDir>
#include <QCoreApplication>

void Playlist::clear() {
    tracks_.clear();
    currentIndex_ = 0;
    while (!backStack_.empty()) backStack_.pop();
    while (!forwardStack_.empty()) forwardStack_.pop();

    shuffleQueue_.clear();
    currentQueuePosition_ = 0;
    maxPositivePosition_ = 0;
    minNegativePosition_ = 0;

    shuffle_ = false;
    repeatMode_ = RepeatMode::None;
}

bool Playlist::setCurrent(size_t i) {
    if (i >= tracks_.size()) return false;

    if (currentIndex_ < tracks_.size() && shuffle_) {
        backStack_.push(currentIndex_);
    }
    while (!forwardStack_.empty()) forwardStack_.pop();

    currentIndex_ = i;

    if (shuffle_) {
        shuffleQueue_[currentQueuePosition_] = i;
    }

    return true;
}

std::optional<Track> Playlist::current() const {
    if (currentIndex_ >= tracks_.size()) return std::nullopt;
    return tracks_[currentIndex_];
}

bool Playlist::next() {
    if (tracks_.empty()) return false;

    // Режим повтора одного трека - используем стандартную логику
    if (repeatMode_ == RepeatMode::One) {
        size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
        return setCurrent(nextIdx);
    }

    // Shuffle режим - специальная логика
    if (shuffle_) {
        return navigateInShuffleQueue(1);
    }

    // СТАНДАРТНЫЙ РЕЖИМ - обычная логика
    size_t nextIdx = (currentIndex_ + 1) % tracks_.size();
    return setCurrent(nextIdx);
}

bool Playlist::prev(qint64 currentPosition) {
    if (tracks_.empty()) return false;

    // Глобальная проверка правила 3 секунд
    if (shouldRestartTrack(currentPosition)) {
        return true;
    }

    // Shuffle режим - специальная логика
    if (shuffle_) {
        return navigateInShuffleQueue(-1);
    }

    // ИСПОЛЬЗУЕМ РОДИТЕЛЬСКИЙ МЕТОД для стандартного режима и режима повтора
    return standardPrevLogic(currentPosition);
}

// РОДИТЕЛЬСКИЙ МЕТОД - стандартная логика навигации назад
// Используется в:
// 1. Стандартном режиме (shuffle = false, repeatMode = None)
// 2. Режиме повтора трека (shuffle = false, repeatMode = One)
bool Playlist::standardPrevLogic(qint64 currentPosition) {
    // Режим повтора одного трека - стандартная циклическая навигация
    if (repeatMode_ == RepeatMode::One) {
        // От первого трека назад = последний трек
        // От любого другого трека назад = предыдущий трек
        size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
        return setCurrent(prevIdx);
    }

    // СТАНДАРТНЫЙ РЕЖИМ (repeatMode = None) - логика с историей

    // Если есть история назад - восстанавливаем предыдущий трек
    if (!backStack_.empty()) {
        forwardStack_.push(currentIndex_);
        currentIndex_ = backStack_.top();
        backStack_.pop();
        return true;
    }

    // Если истории нет (мы в начале навигации) - циклический переход
    // От первого трека назад = последний трек папки
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

    if (direction == 1) {
        addToShuffleQueue(targetPosition);
    } else {
        addToShuffleQueue(targetPosition);
    }

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

    // Проверяем что рейтинг в допустимом диапазоне (0.0 - 5.0)
    if (rating < 0.0 || rating > 5.0) return false;

    // Устанавливаем рейтинг текущему треку
    tracks_[currentIndex_].setTrackRating(rating);

    // Сохраняем рейтинг в файл
    saveRatings();

    return true;
}

void Playlist::loadRatings() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt";

    std::ifstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return; // Файл не существует - это нормально при первом запуске
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string trackPath;
        double rating;

        if (std::getline(iss, trackPath, '|') && (iss >> rating)) {
            // Ищем трек с таким путем и устанавливаем рейтинг
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

// НОВЫЙ МЕТОД: сохранение рейтингов в файл
void Playlist::saveRatings() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt";

    std::ofstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return; // Не удалось создать файл
    }

    // Сохраняем только треки с ненулевым рейтингом
    for (const auto& track : tracks_) {
        if (track.rating() > 0.0) {
            file << track.path() << "|" << track.rating() << "\n";
        }
    }

    file.close();
}
