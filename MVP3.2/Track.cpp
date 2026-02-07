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
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>

#include <QCoreApplication>  // Добавьте эту строку

#include "resource_finder.h"

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)), rating_(rating) {

    // Пытаемся загрузить метаданные из MP3
    if (!loadMetadataFromMP3()) {
        // Если не удалось, используем переданные значения
        qDebug() << "Не удалось загрузить метаданные для:" << QString::fromStdString(path_);
    }
}

QString findDefaultCover() {
    QString appDir = QCoreApplication::applicationDirPath();

    // 1. Ищем рядом с .exe
    QString path1 = QDir::cleanPath(appDir + "/images/default.jpg");
    if (QFile::exists(path1)) {
        return path1;
    }

    // 2. Ищем в папке с иконками
    QString path2 = QDir::cleanPath(appDir + "/icons/default.jpg");
    if (QFile::exists(path2)) {
        return path2;
    }

    // 3. Ищем прямо рядом с .exe
    QString path3 = QDir::cleanPath(appDir + "/default.jpg");
    if (QFile::exists(path3)) {
        return path3;
    }

// 4. Абсолютный путь (только для отладки)
#ifdef QT_DEBUG
    QString path4 = "C:/My_QT/CPP/Alex_Music/Alex-Music/MVP1/images/default.jpg";
    if (QFile::exists(path4)) {
        return path4;
    }
#endif

    return QString();  // Не нашли
}

QImage Track::getCoverImage() const {
    // Пытаемся извлечь обложку из MP3 файла
    QImage cover = extractCoverFromMP3();

    // Если не получилось - загружаем обложку по умолчанию
    if (cover.isNull()) {
        cover = loadDefaultCover();
    }

    return cover;
}

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

// ИСПРАВЛЕННЫЙ МЕТОД
QImage Track::loadDefaultCover() const {
    QString coverPath = ResourceFinder::findDefaultCover();

    if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
        QImage image(coverPath);
        if (!image.isNull()) {
            return image;
        }
    }

    // Если не нашли - создаем серый квадрат
    QImage grayImage(200, 200, QImage::Format_RGB32);
    grayImage.fill(Qt::darkGray);
    return grayImage;
}

std::string Track::getID() const {
    return path_;
}

// Track.cpp
bool Track::loadMetadataFromMP3() {
    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(2000);

    bool metadataLoaded = false;

    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto metadata = player.metaData();

        // Название трека
        QVariant titleVariant = metadata.value(QMediaMetaData::Title);
        if (titleVariant.isValid() && !titleVariant.toString().isEmpty()) {
            title_ = titleVariant.toString().toStdString();
        }

        // Исполнитель
        QVariant authorVariant = metadata.value(QMediaMetaData::Author);
        if (authorVariant.isValid() && !authorVariant.toString().isEmpty()) {
            artist_ = authorVariant.toString().toStdString();
        } else {
            QVariant albumArtistVariant = metadata.value(QMediaMetaData::AlbumArtist);
            if (albumArtistVariant.isValid() && !albumArtistVariant.toString().isEmpty()) {
                artist_ = albumArtistVariant.toString().toStdString();
            }
        }

        // Альбом
        QVariant albumVariant = metadata.value(QMediaMetaData::AlbumTitle);
        if (albumVariant.isValid() && !albumVariant.toString().isEmpty()) {
            album_ = albumVariant.toString().toStdString();
        }

        // Год/дата
        QVariant dateVariant = metadata.value(QMediaMetaData::Date);
        if (dateVariant.isValid() && !dateVariant.toString().isEmpty()) {
            year_ = dateVariant.toString().toStdString();
        }

        // Жанр
        QVariant genreVariant = metadata.value(QMediaMetaData::Genre);
        if (genreVariant.isValid() && !genreVariant.toString().isEmpty()) {
            genre_ = genreVariant.toString().toStdString();
        }

        // Номер трека
        QVariant trackNumberVariant = metadata.value(QMediaMetaData::TrackNumber);
        if (trackNumberVariant.isValid()) {
            bool ok;
            int trackNum = trackNumberVariant.toInt(&ok);
            if (ok) {
                trackNumber_ = trackNum;
            }
        }

        metadataLoaded = true;
        loop.quit();
    });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    player.setSource(QUrl::fromLocalFile(QString::fromStdString(path_)));
    loop.exec();

    return metadataLoaded;
}
