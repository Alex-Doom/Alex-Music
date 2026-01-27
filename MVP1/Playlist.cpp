#include "Playlist.h"
#include <random>        // Стандартная библиотека случайных чисел
#include <fstream>       // Работа с файлами
#include <sstream>       // Строковые потоки
#include <QDir>          // Работа с директориями Qt
#include <QCoreApplication> // Основной класс приложения Qt
// #include "TrackValidator.h"
// #include "BadTrackDialog.h"

void Playlist::clear() {
    tracks_.clear(); // Очищаем вектор треков
    currentIndex_ = 0; // Сбрасываем текущий индекс
    skipInvalidTracks_ = false; // Сбрасываем флаг при очистке

    // Очистка стека истории назад
    while (!backStack_.empty()) backStack_.pop();
    // Очистка стека истории вперед
    while (!forwardStack_.empty()) forwardStack_.pop();

    // Очищаем shuffle очередь
    shuffleQueue_.clear();
    currentQueuePosition_ = 0; // трек отсчета - нулевой, а от него случайные треки
    maxPositivePosition_ = 0; // вперед - положительное,
    minNegativePosition_ = 0; // назад - отрицательное направление

    // сброс флагов режимов
    shuffle_ = false;
    repeatMode_ = RepeatMode::None;
}

// Возвращает текущий трек или std::nullopt если плейлист пуст
std::optional<Track> Playlist::current() const {
    if (currentIndex_ >= tracks_.size()) return std::nullopt;
    return tracks_[currentIndex_]; // Возврат трека по текущему индексу
}

// Безопасная навигация с пропуском битых треков
bool Playlist::safeNavigate(bool forward, int maxAttempts) {
    int attempts = 0;

    // Сохраняем текущий индекс перед попыткой
    size_t originalIndex = currentIndex_;

    while (attempts < maxAttempts) {
        // Пытаемся перейти
        bool success = forward ? nextInternal() : prevInternal();

        if (!success) {
            return false;
        }

        // Если трек сменился - проверяем его
        if (originalIndex != currentIndex_) {
            return true;
        }
        attempts++;
    }

    return false;
}

// Внутренний метод next без рекурсии
bool Playlist::nextInternal() {
    if (tracks_.empty()) return false;

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

// Внутренний метод prev без рекурсии
bool Playlist::prevInternal(qint64 currentPosition) {
    if (tracks_.empty()) return false;

    if (shouldRestartTrack(currentPosition)) {
        return true;
    }

    if (shuffle_) {
        return navigateInShuffleQueue(-1);
    }

    // СТАНДАРТНЫЙ РЕЖИМ (repeatMode = None) - логика с историей
    if (!backStack_.empty()) {
        forwardStack_.push(currentIndex_);
        currentIndex_ = backStack_.top();
        backStack_.pop();
        return true;
    }

    // Циклический переход
    size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
    return setCurrent(prevIdx);
}

// Изменяем публичные методы next() и prev()
bool Playlist::next() {
    return safeNavigate(true);
}

// Изменяем сигнатуру и логику метода prev
bool Playlist::prev(qint64 currentPosition, bool skipThreeSecondRule) {
    if (tracks_.empty()) return false;

    // Проверяем правило 3 секунд, если не отключено
    if (!skipThreeSecondRule && shouldRestartTrack(currentPosition)) {
        qDebug() << "Правило 3 секунд: перезапуск текущего трека";
        return true;
    }

    qDebug() << "Запрос на переход к предыдущему треку, текущий индекс:" << currentIndex_;

    // Пытаемся перейти к предыдущему
    bool success = prevInternal(currentPosition);

    if (success && skipInvalidTracks_) {
        // Если включен режим пропуска битых треков
        auto current = this->current();
        if (current) {
            // Здесь мы не проверяем трек, так как это делает MainWindow
            return true;
        }
    }

    return success;
}

// метод для установки флага пропуска
void Playlist::setSkipInvalidTracks(bool skip) {
    skipInvalidTracks_ = skip;
}

// стандартная логика навигации назад
bool Playlist::standardPrevLogic(qint64 currentPosition) {
    // Режим повтора одного трека - стандартная циклическая навигация
    if (repeatMode_ == RepeatMode::One) {
        // От первого трека плейлиста назад = последний трек
        // От любого другого трека назад = предыдущий трек
        size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
        return setCurrent(prevIdx);
    }

    // СТАНДАРТНЫЙ РЕЖИМ (repeatMode = None) - логика с историей

    // Если есть история назад - восстанавливаем предыдущий трек
    if (!backStack_.empty()) {
        forwardStack_.push(currentIndex_);     // Сохраняем текущий в историю вперед
        currentIndex_ = backStack_.top();      // Восстанавливаем предыдущий
        backStack_.pop();                      // Удаляем из истории назад
        return true;
    }

    // Если истории нет (начало навигации) - циклический переход
    // От первого трека назад = последний трек папки
    size_t prevIdx = (currentIndex_ == 0) ? tracks_.size() - 1 : currentIndex_ - 1;
    return setCurrent(prevIdx);
}

// Генерирует случайный индекс трека (исключая текущий)
size_t Playlist::getRandomTrackIndex() const {
    if (tracks_.size() <= 1) return 0;

    // Создание равномерного распределения
    std::uniform_int_distribution<size_t> dist(0, tracks_.size() - 1);
    size_t randomIndex;

    // Генерируем случайный индекс пока не получим отличный от текущего
    do {
        randomIndex = dist(rng_);
    } while (randomIndex == currentIndex_ && tracks_.size() > 1);

    return randomIndex;
}

// Генерирует случайный индекс исключая указанные треки
size_t Playlist::getRandomTrackIndexExcluding(const std::vector<size_t>& excluded) const {
    if (tracks_.empty()) return 0;
    if (tracks_.size() <= excluded.size()) return 0;

    std::uniform_int_distribution<size_t> dist(0, tracks_.size() - 1);
    size_t randomIndex;

    // Генерируем пока не найдем трек не в списке исключенных
    do {
        randomIndex = dist(rng_);
        // Проверяем не исключен ли трек
        bool isExcluded = std::find(excluded.begin(), excluded.end(), randomIndex) != excluded.end();
        if (!isExcluded) {
            break;
        }
    } while (true);

    return randomIndex;
}

// Добавление трека в shuffle очередь по указанной позиции
void Playlist::addToShuffleQueue(int queuePosition) {
    // Собираем список уже использованных треков
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

// Навигация в shuffle очереди
bool Playlist::navigateInShuffleQueue(int direction) {
    int targetPosition = currentQueuePosition_ + direction; // Вычисление позиции

    // есть ли трек в целевой позиции
    auto it = shuffleQueue_.find(targetPosition);
    if (it != shuffleQueue_.end()) { // Если есть - переходим к нему
        currentQueuePosition_ = targetPosition;
        currentIndex_ = it->second;
        return true;
    }

    if (direction == 1) { // Если трека нет в целевой позиции - добавляем новый
        addToShuffleQueue(targetPosition); // Для движения вперед
    } else {
        addToShuffleQueue(targetPosition); // Для движения назад
    }

    // Обновление текущей позиции и индекса
    currentQueuePosition_ = targetPosition;
    currentIndex_ = shuffleQueue_[targetPosition];

    return true;
}

// Включение/выключение случайного воспроизведения
void Playlist::setShuffle(bool enabled) {
    if (enabled && !shuffle_) {
        // Включение shuffle - инициализирование очереди
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        maxPositivePosition_ = 0;
        minNegativePosition_ = 0;

        // Установка текущего трек начальным
        shuffleQueue_[0] = currentIndex_;

    } else if (!enabled && shuffle_) {
        shuffleQueue_.clear(); // Выключение shuffle - очистка очереди
        currentQueuePosition_ = 0;
        maxPositivePosition_ = 0;
        minNegativePosition_ = 0;
    }

    shuffle_ = enabled;
}

// Сброс истории shuffle
void Playlist::resetShuffleHistory() {
    shuffleQueue_.clear();
    currentQueuePosition_ = 0;
    maxPositivePosition_ = 0;
    minNegativePosition_ = 0;

    if (shuffle_) {
        shuffleQueue_[0] = currentIndex_;
    }
}

// Проверка перезапуска текущего трека (правило 3 секунд)
bool Playlist::shouldRestartTrack(qint64 currentPosition) const {
    return currentPosition > 3000;
}

// рейтинг текущему треку
bool Playlist::setCurrentTrackRating(double rating) {
    if (currentIndex_ >= tracks_.size()) return false;

    // Проверяем что рейтинг в допустимом диапазоне (0.0 - 5.0)
    if (rating < 0.0 || rating > 5.0) return false;

    // Устанавливаем рейтинг текущему треку
    tracks_[currentIndex_].setTrackRating(rating);

    // Сохранение рейтинга в файл
    saveRatings();

    return true;
}

// Загрузка рейтингов из файла
void Playlist::loadRatings() {
    // Получение директории приложения
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt"; // Формирование пути к файлу

    // Открытие файла для чтения
    std::ifstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    // Читаем файл построчно
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string trackPath;
        double rating;

        // Разбираем строку: путь|рейтинг
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

// сохранение рейтингов в файл
void Playlist::saveRatings() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString ratingsFile = appDir + "/ratings.txt";

    // Открываем файл для записи
    std::ofstream file(ratingsFile.toStdString());
    if (!file.is_open()) {
        return; // Не удалось создать файл
    }

    // Сохранение только треков с ненулевым рейтингом
    for (const auto& track : tracks_) {
        if (track.rating() > 0.0) {
            file << track.path() << "|" << track.rating() << "\n";
        }
    }

    file.close();
}

// Установка текущего трека по индексу
bool Playlist::setCurrent(size_t i, bool resetShuffle) {
    if (i >= tracks_.size()) return false;

    // Сохранение текущего трека в историю если shuffle включен
    if (currentIndex_ < tracks_.size() && shuffle_) {
        backStack_.push(currentIndex_);
    }
    // Очищение истории вперед при смене трека
    while (!forwardStack_.empty()) forwardStack_.pop();

    currentIndex_ = i; // Установка нового индекса

    // Если нужно сбросить shuffle - устанавление якоря
    if (resetShuffle) {
        setCurrentAsShuffleAnchor();
    }

    // Обновление shuffle очереди, если shuffle включен
    if (shuffle_) {
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        shuffleQueue_[0] = i;

        // Обновляем якорь если нужно
        if (resetShuffle) {
            shuffleAnchorIndex_ = i;
        }
    }

    return true;
}

// Установка текущего трека как якоря - трека отсчета - для shuffle
void Playlist::setCurrentAsShuffleAnchor() {
    if (tracks_.empty()) return;

    shuffleAnchorIndex_ = currentIndex_;

    if (shuffle_) {
        // Полностью перестройка shuffle очереди от нового трека
        shuffleQueue_.clear();
        currentQueuePosition_ = 0;
        shuffleQueue_[0] = currentIndex_;

        // Очистка истории навигации
        while (!backStack_.empty()) backStack_.pop();
        while (!forwardStack_.empty()) forwardStack_.pop();
    }
}

// Проверка возможности перехода
bool Playlist::canNavigate(bool forward) const {
    if (tracks_.empty()) return false;

    if (forward) {
        if (repeatMode_ == RepeatMode::One) return true;
        return tracks_.size() > 1 || repeatMode_ == RepeatMode::None;
    } else {
        if (repeatMode_ == RepeatMode::One) return true;
        return tracks_.size() > 1 || !backStack_.empty();
    }
}
