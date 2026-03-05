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

    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000); // Таймаут 3 секунды

    // Обработчик метаданных
    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        // Получаем все доступные метаданные
        auto metaData = player.metaData();

        // Заголовок
        metadata.title = metaData.value(QMediaMetaData::Title).toString();
        if (metadata.title.isEmpty()) {
            // Если нет заголовка в метаданных, используем имя файла
            QFileInfo fileInfo(filePath);
            metadata.title = fileInfo.baseName();
        }

        // Исполнитель
        metadata.artist = metaData.value(QMediaMetaData::Author).toString();
        if (metadata.artist.isEmpty()) {
            metadata.artist = metaData.value(QMediaMetaData::AlbumArtist).toString();
        }

        // Альбом
        metadata.album = metaData.value(QMediaMetaData::AlbumTitle).toString();

        // Жанр
        metadata.genre = metaData.value(QMediaMetaData::Genre).toString();

        // Год
        QVariant yearVariant = metaData.value(QMediaMetaData::Date);
        if (yearVariant.isValid()) {
            metadata.year = yearVariant.toString();
        } else {
            // Попробуем получить из даты
            QVariant dateVariant = metaData.value(QMediaMetaData::Date);
            if (dateVariant.isValid()) {
                QDate date = dateVariant.toDate();
                if (date.isValid()) {
                    metadata.year = QString::number(date.year());
                }
            }
        }

        // Номер трека
        metadata.trackNumber = metaData.value(QMediaMetaData::TrackNumber).toInt();

        // Длительность
        metadata.duration = player.duration() / 1000; // в секундах

        // Обложка
        QVariant coverData = metaData.value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) {
            metadata.coverImage = coverData.value<QImage>();
        }

        if (metadata.coverImage.isNull()) {
            QVariant thumbnailData = metaData.value(QMediaMetaData::ThumbnailImage);
            if (thumbnailData.isValid()) {
                metadata.coverImage = thumbnailData.value<QImage>();
            }
        }

        loop.quit();
    });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Устанавливаем источник
    player.setSource(QUrl::fromLocalFile(filePath));

    // Ждем завершения
    loop.exec();

    return metadata;
}

QImage Mp3Metadata::extractCover(const QString& filePath) {
    return fromFile(filePath).coverImage;
}
