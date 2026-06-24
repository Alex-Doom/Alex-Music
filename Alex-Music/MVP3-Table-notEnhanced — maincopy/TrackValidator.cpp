#include "TrackValidator.h"
#include <QFileInfo>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>

TrackValidator::TrackValidator(QObject* parent) : QObject(parent) {}

bool TrackValidator::validate(const QString& path) {
    error_.clear();
    QFileInfo info(path);

    if (!info.exists()) {
        error_ = "Файл не найден";
        return false;
    }
    if (info.size() == 0) {
        error_ = "Файл пуст";
        return false;
    }
    if (!path.endsWith(".mp3", Qt::CaseInsensitive)) {
        error_ = "Не MP3 файл";
        return false;
    }

    qint64 duration = 0;
    if (!checkDuration(path, duration) || duration < 1000) {
        error_ = "Невозможно воспроизвести (повреждён?)";
        return false;
    }

    return true;
}

bool TrackValidator::checkDuration(const QString& path, qint64& duration) {
    QMediaPlayer player;
    QAudioOutput audio;
    player.setAudioOutput(&audio);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);

    bool gotDuration = false;
    connect(&player, &QMediaPlayer::durationChanged, [&](qint64 d) {
        duration = d;
        gotDuration = true;
        loop.quit();
    });
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    player.setSource(QUrl::fromLocalFile(path));
    loop.exec();
    return gotDuration;
}
