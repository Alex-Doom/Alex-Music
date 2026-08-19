// Track.cpp
#include "Track.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QImage>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>
#include <QRegularExpression>

#include "resource_finder.h"
#include "Mp3Metadata.h"
#include "FastMetadataReader.h"
#include "FastTagReader.h"

Track::Track(const std::string& path) : path_(path) {
    artist_ = "Unknown Artist";
    title_ = "Unknown Title";
    album_ = "Unknown Album";
    genre_ = "Unknown";
    year_ = "Unknown";
    rating_ = 0.0;
}

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)), rating_(rating) {
    genre_ = "Unknown";
    year_ = "Unknown";
}

void Track::loadMetadata() {
    qDebug() << "Загрузка метаданных для:" << QString::fromStdString(path_);

    // === ШАГ 1: Всегда начинаем с имени файла (надёжный источник для русских названий) ===
    loadMetadataFast();
    QString fileNameArtist = QString::fromStdString(artist_);
    QString fileNameTitle = QString::fromStdString(title_);

    // === ШАГ 2: Пробуем получить метаданные из TagLib ===
    TrackMetadata metadata = FastTagReader::readMetadata(QString::fromStdString(path_));

    // === ШАГ 3: Проверяем, не содержат ли метаданные кракозябры ===
    auto hasGarbage = [](const QString& s) -> bool {
        if (s.isEmpty()) return true; // Пустое тоже считаем "мусором"
        for (const QChar& c : s) {
            ushort u = c.unicode();
            // Latin-1 Supplement (U+00C0-U+00FF) = признак mojibake от CP1251
            if (u >= 0x00C0 && u <= 0x00FF) {
                return true;
            }
        }
        return false;
    };

    // Используем TagLib только если данные валидные (без кракозябр)
    if (!hasGarbage(metadata.title)) {
        title_ = metadata.title.toStdString();
        qDebug() << "  Title из тегов:" << metadata.title;
    } else {
        qDebug() << "  Title из тегов содержит кракозябры, используем имя файла:" << fileNameTitle;
    }

    if (!hasGarbage(metadata.artist)) {
        artist_ = metadata.artist.toStdString();
        qDebug() << "  Artist из тегов:" << metadata.artist;
    } else {
        qDebug() << "  Artist из тегов содержит кракозябры, используем имя файла:" << fileNameArtist;
    }

    // Альбом, жанр, год — используем из тегов только если валидны
    if (!hasGarbage(metadata.album)) {
        album_ = metadata.album.toStdString();
        qDebug() << "  Album из тегов:" << metadata.album;
    }

    if (!hasGarbage(metadata.genre)) {
        genre_ = metadata.genre.toStdString();
        qDebug() << "  Genre из тегов:" << metadata.genre;
    }

    if (!metadata.year.isEmpty()) {
        year_ = metadata.year.toStdString();
        qDebug() << "  Year из тегов:" << metadata.year;
    }

    // Обложка всегда из TagLib
    if (!metadata.coverImage.isNull()) {
        coverImage_ = metadata.coverImage;
        coverLoaded_ = true;
        qDebug() << "  Cover: loaded from taglib";
    }

    metadataLoaded_ = true;
}

QImage Track::getCoverImage() const {
    if (coverLoaded_ && !coverImage_.isNull()) {
        return coverImage_;
    }

    // Используем ТОЛЬКО FastTagReader для обложки
    QImage cover = FastTagReader::readMetadata(QString::fromStdString(path_)).coverImage;
    if (!cover.isNull()) {
        return cover;
    }

    return loadDefaultCover();
}

QImage Track::extractCoverFromMP3() const {
    return Mp3Metadata::extractCover(QString::fromStdString(path_));
}

QImage Track::loadDefaultCover() const {
    QString coverPath = ResourceFinder::findDefaultCover();

    if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
        QImage image(coverPath);
        if (!image.isNull()) {
            return image;
        }
    }

    QImage grayImage(200, 200, QImage::Format_RGB32);
    grayImage.fill(Qt::darkGray);
    return grayImage;
}

std::string Track::getID() const {
    return path_;
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
    metadataLoaded_ = true;
}

void Track::loadMetadataAsync() {
    // Используем FastMetadataReader для асинхронной загрузки
    TrackMetadata metadata = FastMetadataReader::instance().getMetadata(
        QString::fromStdString(path_));

    if (!metadata.title.isEmpty()) {
        title_ = metadata.title.toStdString();
    }
    if (!metadata.artist.isEmpty()) {
        artist_ = metadata.artist.toStdString();
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
    if (!metadata.coverImage.isNull()) {
        coverImage_ = metadata.coverImage;
        coverLoaded_ = true;
    }
}
