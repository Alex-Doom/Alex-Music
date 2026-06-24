#include "Playlist.h"
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <fstream>

Playlist::Playlist(QObject* parent) : QObject(parent) {}

void Playlist::add(const Track& track) {
    tracks_.push_back(track);
}

void Playlist::clear() {
    tracks_.clear();
    currentIndex_ = 0;
    shuffleOrder_.clear();
    shufflePos_ = 0;
}

std::optional<Track> Playlist::current() const {
    if (tracks_.empty() || currentIndex_ >= tracks_.size())
        return std::nullopt;
    return tracks_[currentIndex_];
}

bool Playlist::setCurrent(size_t index) {
    if (index >= tracks_.size()) return false;
    currentIndex_ = index;
    emit currentChanged(index);
    return true;
}

bool Playlist::next() {
    if (tracks_.empty()) return false;

    if (repeatMode_ == RepeatMode::One) {
        return true; // Остаёмся на текущем
    }

    if (shuffle_) {
        ensureShuffleOrder();
        if (++shufflePos_ >= shuffleOrder_.size()) {
            shufflePos_ = 0; // Цикл
        }
        currentIndex_ = shuffleOrder_[shufflePos_];
    } else {
        currentIndex_ = (currentIndex_ + 1) % tracks_.size();
    }

    emit currentChanged(currentIndex_);
    return true;
}

bool Playlist::prev(qint64 currentPositionMs) {
    if (tracks_.empty()) return false;

    // Правило 3 секунд
    if (currentPositionMs > 3000) {
        return true; // Сигнал перезапустить текущий
    }

    if (repeatMode_ == RepeatMode::One) {
        return true;
    }

    if (shuffle_) {
        ensureShuffleOrder();
        if (shufflePos_ > 0) {
            shufflePos_--;
            currentIndex_ = shuffleOrder_[shufflePos_];
        } else {
            shufflePos_ = shuffleOrder_.size() - 1;
            currentIndex_ = shuffleOrder_[shufflePos_];
        }
    } else {
        currentIndex_ = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
    }

    emit currentChanged(currentIndex_);
    return true;
}

void Playlist::setShuffle(bool enabled) {
    if (enabled && !shuffle_) {
        generateShuffleOrder();
        shufflePos_ = 0;
        // Текущий трек становится первым в shuffle
        for (size_t i = 0; i < shuffleOrder_.size(); ++i) {
            if (shuffleOrder_[i] == currentIndex_) {
                std::swap(shuffleOrder_[0], shuffleOrder_[i]);
                break;
            }
        }
    }
    shuffle_ = enabled;
}

void Playlist::generateShuffleOrder() {
    shuffleOrder_.resize(tracks_.size());
    for (size_t i = 0; i < tracks_.size(); ++i) shuffleOrder_[i] = i;
    std::shuffle(shuffleOrder_.begin(), shuffleOrder_.end(), rng_);
}

void Playlist::ensureShuffleOrder() {
    if (shuffleOrder_.size() != tracks_.size()) {
        generateShuffleOrder();
    }
}

void Playlist::setRating(double rating) {
    if (currentIndex_ < tracks_.size()) {
        tracks_[currentIndex_].setRating(rating);
        emit ratingChanged(currentIndex_, rating);
        saveRatings();
    }
}

void Playlist::loadRatings() {
    QString path = QCoreApplication::applicationDirPath() + "/ratings.dat";
    std::ifstream file(path.toStdString());
    if (!file) return;

    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('|');
        if (pos == std::string::npos) continue;

        QString filePath = QString::fromStdString(line.substr(0, pos));
        double rating = std::stod(line.substr(pos + 1));

        for (auto& track : tracks_) {
            if (track.uniqueId() == filePath) {
                track.setRating(rating);
                break;
            }
        }
    }
}

void Playlist::saveRatings() {
    QString path = QCoreApplication::applicationDirPath() + "/ratings.dat";
    std::ofstream file(path.toStdString());
    if (!file) return;

    for (const auto& track : tracks_) {
        if (track.rating() > 0) {
            file << track.uniqueId().toStdString() << "|" << track.rating() << "\n";
        }
    }
}

void Playlist::applySort(SortMode mode) {
    sortMode_ = mode;
    // Сортировка применяется только для отображения, не для воспроизведения
    // Это сохраняет логику shuffle независимой от сортировки
}
