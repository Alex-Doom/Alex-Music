#include "TrackValidator.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>
#include <QEventLoop>
#include <QTimer>

TrackValidator::TrackValidator(QObject* parent) : QObject(parent) {}

bool TrackValidator::validateTrack(const QString& filePath) {
    lastError_.clear();

    qDebug() << "validateTrack: проверяем" << filePath;

    // Проверяем существование файла
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        lastError_ = "Файл не существует";
        qDebug() << "  Файл не существует";
        return false;
    }

    // Проверяем размер файла
    if (fileInfo.size() == 0) {
        lastError_ = "Файл пустой (0 байт)";
        qDebug() << "  Файл пустой";
        return false;
    }

    // Проверяем расширение
    if (!filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        lastError_ = "Неверный формат файла (должен быть .mp3)";
        qDebug() << "  Не MP3 файл";
        return false;
    }

    // Проверяем длительность
    qint64 duration = getDuration(filePath);
    qDebug() << "  Длительность:" << duration << "мс";

    if (duration <= 0) {
        lastError_ = "Невозможно определить длительность трека";
        qDebug() << "  Не удалось определить длительность";
        return false;
    }

    if (duration < 1000) { // Меньше 1 секунды
        lastError_ = "Трек слишком короткий";
        qDebug() << "  Трек слишком короткий";
        return false;
    }

    qDebug() << "  Трек валиден!";
    return true;
}

qint64 TrackValidator::getDuration(const QString& filePath) {
    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(5000); // Таймаут 5 секунды

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
