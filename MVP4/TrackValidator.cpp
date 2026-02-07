#include "TrackValidator.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>

TrackValidator::TrackValidator(QObject* parent) : QObject(parent) {}

// TrackValidator.cpp - добавьте эти проверки в validateTrack
bool TrackValidator::validateTrack(const QString& filePath) {
    lastError_.clear();

    // Проверяем существование файла
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        lastError_ = "Файл не существует";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    // Проверяем размер файла
    if (fileInfo.size() == 0) {
        lastError_ = "Файл пустой (0 байт)";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    // Проверяем доступность файла
    if (!fileInfo.isReadable()) {
        lastError_ = "Файл недоступен для чтения";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    // Проверяем расширение
    if (!filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        lastError_ = "Неверный формат файла (должен быть .mp3)";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    // Проверяем длительность
    qint64 duration = getDuration(filePath);
    if (duration <= 0) {
        lastError_ = "Невозможно определить длительность трека";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    if (duration < 1000) { // Меньше 1 секунды
        lastError_ = "Трек слишком короткий (меньше 1 секунды)";
        emit validationFailed(filePath, lastError_);
        return false;
    }

    // Дополнительная проверка через QMediaPlayer
    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(2000); // Таймаут 2 секунды

    bool isPlayable = false;
    QString errorString;

    QObject::connect(&player, &QMediaPlayer::errorOccurred,
                     [&](QMediaPlayer::Error error, const QString& errorStr) {
                         errorString = errorStr;
                         loop.quit();
                     });

    QObject::connect(&player, &QMediaPlayer::mediaStatusChanged,
                     [&](QMediaPlayer::MediaStatus status) {
                         if (status == QMediaPlayer::LoadedMedia ||
                             status == QMediaPlayer::BufferedMedia) {
                             isPlayable = true;
                             loop.quit();
                         } else if (status == QMediaPlayer::InvalidMedia) {
                             errorString = "Неверный медиаформат";
                             loop.quit();
                         }
                     });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    player.setSource(QUrl::fromLocalFile(filePath));

    loop.exec();

    if (!isPlayable) {
        lastError_ = errorString.isEmpty() ? "Трек поврежден" : errorString;
        emit validationFailed(filePath, lastError_);
        return false;
    }

    return true;
}

qint64 TrackValidator::getDuration(const QString& filePath) {
    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000); // Таймаут 3 секунды

    qint64 duration = 0;

    // Подключаем сигналы
    QObject::connect(&player, &QMediaPlayer::durationChanged,
                     [&](qint64 newDuration) {
                         duration = newDuration;
                         loop.quit();
                     });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    player.setSource(QUrl::fromLocalFile(filePath));

    loop.exec();

    return duration;
}
