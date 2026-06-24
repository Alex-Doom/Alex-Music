#include "Mp3Metadata.h"
#include <QFileInfo>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QMediaMetaData>

Mp3Metadata Mp3Metadata::fromFile(const QString& filePath) {
    Mp3Metadata metadata;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        metadata.title = fileInfo.baseName();
        metadata.artist = "Unknown Artist";
        metadata.album = "Unknown Album";
        return metadata;
    }

    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);

    bool metadataReceived = false;

    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto metaData = player.metaData();

        metadata.title = metaData.value(QMediaMetaData::Title).toString();
        if (metadata.title.isEmpty()) {
            metadata.title = metaData.value(QMediaMetaData::AlbumTitle).toString();
        }

        metadata.artist = metaData.value(QMediaMetaData::Author).toString();
        if (metadata.artist.isEmpty()) {
            metadata.artist = metaData.value(QMediaMetaData::AlbumArtist).toString();
        }
        if (metadata.artist.isEmpty()) {
            metadata.artist = metaData.value(QMediaMetaData::ContributingArtist).toString();
        }

        metadata.album = metaData.value(QMediaMetaData::AlbumTitle).toString();
        metadata.genre = metaData.value(QMediaMetaData::Genre).toString();

        QVariant yearVariant = metaData.value(QMediaMetaData::Date);
        if (yearVariant.isValid()) {
            metadata.year = yearVariant.toString();
            if (metadata.year.length() > 4) metadata.year = metadata.year.left(4);
        }

        QVariant trackVariant = metaData.value(QMediaMetaData::TrackNumber);
        if (trackVariant.isValid()) metadata.trackNumber = trackVariant.toInt();

        QVariant coverData = metaData.value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) metadata.coverImage = coverData.value<QImage>();

        if (metadata.coverImage.isNull()) {
            QVariant thumbnailData = metaData.value(QMediaMetaData::ThumbnailImage);
            if (thumbnailData.isValid()) metadata.coverImage = thumbnailData.value<QImage>();
        }

        metadataReceived = true;
        loop.quit();
    });

    QObject::connect(&player, &QMediaPlayer::durationChanged, [&](qint64 duration) {
        metadata.duration = duration / 1000;
    });

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (!metadataReceived) loop.quit();
    });

    player.setSource(QUrl::fromLocalFile(filePath));
    loop.exec();

    // Fallback на имя файла
    if (!metadataReceived || metadata.title.isEmpty()) {
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

        if (parts.size() >= 2) {
            metadata.artist = parts[0].trimmed();
            metadata.title = parts[1].trimmed();
        } else {
            metadata.title = baseName.trimmed();
            metadata.artist = "Unknown Artist";
        }
        metadata.album = "Unknown Album";
        metadata.genre = "Unknown";
    }

    // Очистка заголовка
    while (!metadata.title.isEmpty() &&
           (metadata.title[0].isDigit() || metadata.title[0] == '_' ||
            metadata.title[0] == '.' || metadata.title[0] == '-' || metadata.title[0] == ' ')) {
        metadata.title.remove(0, 1);
    }

    if (metadata.artist.isEmpty()) metadata.artist = "Unknown Artist";
    if (metadata.title.isEmpty()) metadata.title = "Unknown Title";

    return metadata;
}

QImage Mp3Metadata::extractCover(const QString& filePath) {
    return fromFile(filePath).coverImage;
}
