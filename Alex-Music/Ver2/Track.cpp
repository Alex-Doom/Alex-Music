#include "Track.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QImage>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QBuffer>
#include <QEventLoop>
#include <QTimer>

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)), rating_(rating) {}

// ОСНОВНОЙ МЕТОД: только 2 варианта - из MP3 или default.jpg
QImage Track::getCoverImage() const {
    // Пытаемся извлечь обложку из MP3 файла
    QImage cover = extractCoverFromMP3();

    // Если не получилось - используем обложку по умолчанию
    if (cover.isNull()) {
        cover = loadDefaultCover();
    }

    return cover;
}

// МЕТОД 1: извлечение обложки из MP3
QImage Track::extractCoverFromMP3() const {
    QImage cover;

    QMediaPlayer player;
    player.setSource(QUrl::fromLocalFile(QString::fromStdString(path_)));

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        QVariant coverData = player.metaData().value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) {
            cover = coverData.value<QImage>();
            if (!cover.isNull()) {
                loop.quit();
            }
        }

        if (cover.isNull()) {
            QVariant thumbnailData = player.metaData().value(QMediaMetaData::ThumbnailImage);
            if (thumbnailData.isValid()) {
                cover = thumbnailData.value<QImage>();
                if (!cover.isNull()) {
                    loop.quit();
                }
            }
        }
    });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(500);

    loop.exec();

    return cover;
}

// МЕТОД 2: загрузка обложки по умолчанию
QImage Track::loadDefaultCover() const {
    // Жестко заданный путь к обложке по умолчанию
    QString defaultCoverPath = "C:/My_QT/CPP/Alex_Music/work/untitled/build/Desktop_Qt_6_9_3_MinGW_64_bit-Debug/default.jpg";

    // Пытаемся загрузить default.jpg
    if (QFile::exists(defaultCoverPath)) {
        QImage defaultCover(defaultCoverPath);
        if (!defaultCover.isNull()) {
            return defaultCover;
        }
    }

    // Если default.jpg не найден или не загрузился - возвращаем пустое изображение
    return QImage();
}

std::string Track::getID() const {
    return path_;
}
