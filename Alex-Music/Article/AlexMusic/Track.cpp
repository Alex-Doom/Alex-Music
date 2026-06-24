// Track.cpp
#include "Track.h"
#include <QFileInfo>
#include <QDebug>
#include "Mp3Metadata.h"
#include "FastMetadataReader.h"

Track::Track() {
    generateId();
    artist_ = "Unknown Artist";
    title_ = "Unknown Title";
    album_ = "Unknown Album";
    genre_ = "Unknown";
    year_ = "Unknown";
}

Track::Track(const std::string& filePath) : path_(filePath) {
    generateId();
    artist_ = "Unknown Artist";
    title_ = "Unknown Title";
    album_ = "Unknown Album";
    genre_ = "Unknown";
    year_ = "Unknown";
}

Track::Track(const std::string& filePath, const std::string& artist,
             const std::string& title, const std::string& album, double rating)
    : path_(filePath), artist_(artist), title_(title), album_(album), rating_(rating) {
    generateId();
    genre_ = "Unknown";
    year_ = "Unknown";
}

void Track::generateId() {
    // Используем путь к файлу как основу для ID
    id_ = QUuid::createUuid().toString();
}

void Track::loadMetadataFast() {
    // Быстрая загрузка только из имени файла
    QFileInfo fileInfo(QString::fromStdString(path_));
    QString baseName = fileInfo.baseName();

    // Удаляем расширение .mp3 если осталось
    baseName.remove(".mp3", Qt::CaseInsensitive);

    // Формат: "Artist - Title.mp3"
    QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

    if (parts.size() >= 2) {
        QString artist = parts[0].trimmed();
        QString title = parts[1].trimmed();

        // Очищаем от цифр в начале только название, исполнителя не трогаем
        while (!title.isEmpty() && title[0].isDigit()) {
            title.remove(0, 1);
        }
        // Удаляем только первый символ если это подчеркивание, точка или дефис
        if (!title.isEmpty() && (title[0] == '_' || title[0] == '.' || title[0] == '-' || title[0] == ' ')) {
            title.remove(0, 1);
        }

        artist_ = artist.toStdString();
        title_ = title.toStdString();

        qDebug() << "Быстрая загрузка:" << QString::fromStdString(path_)
                 << "-> Artist:" << artist << "Title:" << title;
    } else {
        QString title = baseName.trimmed();

        // Очищаем от цифр в начале
        while (!title.isEmpty() && title[0].isDigit()) {
            title.remove(0, 1);
        }
        // Удаляем подчеркивания, точки, дефисы в начале
        while (!title.isEmpty() && (title[0] == '_' || title[0] == '.' || title[0] == '-' || title[0] == ' ')) {
            title.remove(0, 1);
        }

        title_ = title.toStdString();
        artist_ = "Unknown Artist";

        qDebug() << "Быстрая загрузка (только название):" << QString::fromStdString(path_)
                 << "-> Title:" << title;
    }

    album_ = "Unknown Album";
    genre_ = "Unknown";
    year_ = "Unknown";
    hasMetadata_ = false;
}

void Track::loadMetadataFull() {
    qDebug() << "Полная загрузка метаданных для:" << QString::fromStdString(path_);

    Mp3Metadata metadata = Mp3Metadata::fromFile(QString::fromStdString(path_));

    if (!metadata.title.isEmpty()) {
        title_ = metadata.title.toStdString();
        qDebug() << "  Title:" << metadata.title;
    }

    if (!metadata.artist.isEmpty()) {
        artist_ = metadata.artist.toStdString();
        qDebug() << "  Artist:" << metadata.artist;
    }

    if (!metadata.album.isEmpty()) {
        album_ = metadata.album.toStdString();
        qDebug() << "  Album:" << metadata.album;
    }

    if (!metadata.genre.isEmpty()) {
        genre_ = metadata.genre.toStdString();
        qDebug() << "  Genre:" << metadata.genre;
    }

    if (!metadata.year.isEmpty()) {
        year_ = metadata.year.toStdString();
        qDebug() << "  Year:" << metadata.year;
    }

    if (!metadata.coverImage.isNull()) {
        coverImage_ = metadata.coverImage;
        coverLoaded_ = true;
        qDebug() << "  Cover: loaded";
    }

    hasMetadata_ = true;
}

QImage Track::coverImage() const {
    if (coverLoaded_ && !coverImage_.isNull()) {
        return coverImage_;
    }

    QImage cover = extractCoverFromMP3();
    if (!cover.isNull()) {
        return cover;
    }

    return loadDefaultCover();
}

void Track::setCoverImage(const QImage& image) {
    coverImage_ = image;
    coverLoaded_ = true;
}

QImage Track::extractCoverFromMP3() const {
    return Mp3Metadata::extractCover(QString::fromStdString(path_));
}

QImage Track::loadDefaultCover() const {
    QImage grayImage(200, 200, QImage::Format_RGB32);
    grayImage.fill(Qt::darkGray);
    return grayImage;
}
