#include "Track.h"
#include <QFileInfo>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>
#include <QMediaMetaData>
#include <QDebug>

Track::Track(const QString& filePath) : path_(filePath) {
    parseFromFilename();
}

void Track::parseFromFilename() {
    QFileInfo info(path_);
    QString base = info.baseName();

    // Формат: "Artist - Title" или "Title"
    int sep = base.indexOf(" - ");
    if (sep > 0) {
        artist_ = base.left(sep).trimmed();
        title_ = base.mid(sep + 3).trimmed();
    } else {
        title_ = base.trimmed();
    }

    // Очистка номеров треков в начале (01. 02_ и т.д.)
    QRegularExpression re("^(\\d+)[\\.\\-_\\s]+");
    title_.remove(re);
}

void Track::loadFullMetadata() {
    QMediaPlayer player;
    QAudioOutput audio;
    player.setAudioOutput(&audio);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(2000); // 2 секунды таймаут

    connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto meta = player.metaData();

        if (!meta.value(QMediaMetaData::Title).toString().isEmpty())
            title_ = meta.value(QMediaMetaData::Title).toString();
        if (!meta.value(QMediaMetaData::Author).toString().isEmpty())
            artist_ = meta.value(QMediaMetaData::Author).toString();
        if (!meta.value(QMediaMetaData::AlbumTitle).toString().isEmpty())
            album_ = meta.value(QMediaMetaData::AlbumTitle).toString();
        if (!meta.value(QMediaMetaData::Genre).toString().isEmpty())
            genre_ = meta.value(QMediaMetaData::Genre).toString();

        // Год/Дата
        auto date = meta.value(QMediaMetaData::Date);
        if (date.isValid()) {
            year_ = date.toDate().toString("yyyy");
        }

        // Обложка
        auto coverData = meta.value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) {
            cover_ = coverData.value<QImage>();
            coverLoaded_ = true;
        }

        duration_ = player.duration() / 1000;
        loop.quit();
    });

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    player.setSource(QUrl::fromLocalFile(path_));
    loop.exec();
}

QImage Track::cover() const {
    if (coverLoaded_) return cover_;

    // Ленивая загрузка обложки
    QMediaPlayer player;
    player.setSource(QUrl::fromLocalFile(path_));

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);

    connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto data = player.metaData().value(QMediaMetaData::CoverArtImage);
        if (data.isValid()) {
            cover_ = data.value<QImage>();
            coverLoaded_ = true;
        }
        loop.quit();
    });

    loop.exec();
    return cover_;
}
