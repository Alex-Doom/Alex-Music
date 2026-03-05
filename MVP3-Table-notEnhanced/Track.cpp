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
#include <QCoreApplication>  // Добавьте эту строку

#include "resource_finder.h"
#include "Mp3Metadata.h"

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, std::string genre, std::string year,
             double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)),
    genre_(std::move(genre)), year_(std::move(year)), rating_(rating) {}


void Track::loadMetadata() {
    QString filePath = QString::fromStdString(path_);

    // Быстрая проверка файла
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qDebug() << "Файл не существует:" << filePath;
        return;
    }

    if (fileInfo.size() == 0) {
        qDebug() << "Файл пустой:" << filePath;
        return;
    }

    qDebug() << "Загружаем метаданные для:" << fileInfo.fileName();

    try {
        Mp3Metadata metadata = Mp3Metadata::fromFile(filePath);

        qDebug() << "  Полученные метаданные:";
        qDebug() << "    Title:" << metadata.title;
        qDebug() << "    Artist:" << metadata.artist;
        qDebug() << "    Album:" << metadata.album;
        qDebug() << "    Genre:" << metadata.genre;
        qDebug() << "    Year:" << metadata.year;

        // Обновляем только если метаданные не пустые
        if (!metadata.title.isEmpty()) {
            title_ = metadata.title.toStdString();
        } else {
            // Если нет заголовка - используем имя файла
            QString baseName = fileInfo.baseName();
            title_ = baseName.toStdString();
            qDebug() << "    Используем имя файла как заголовок:" << baseName;
        }

        if (!metadata.artist.isEmpty()) {
            artist_ = metadata.artist.toStdString();
        } else {
            artist_ = "Unknown Artist";
            qDebug() << "    Исполнитель не найден, используем Unknown Artist";
        }

        if (!metadata.album.isEmpty()) {
            album_ = metadata.album.toStdString();
        }

        if (!metadata.genre.isEmpty()) {
            genre_ = metadata.genre.toStdString();
        }

        if (!metadata.year.isEmpty()) {
            year_ = metadata.year.toStdString();
        }
    } catch (const std::exception& e) {
        qDebug() << "Ошибка при загрузке метаданных:" << e.what();

        // В случае ошибки используем имя файла
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);
        QString artist = parts.value(0, "Unknown Artist");
        QString title = parts.value(1, baseName);

        title_ = title.toStdString();
        artist_ = artist.toStdString();
        album_ = "";
        genre_ = "";
        year_ = "";
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
    timer.start(1500);

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
