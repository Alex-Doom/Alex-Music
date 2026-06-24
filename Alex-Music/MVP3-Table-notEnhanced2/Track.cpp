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

#include "resource_finder.h"
#include "Mp3Metadata.h"
#include "FastMetadataReader.h"

Track::Track(const std::string& path) : path_(path) {}

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)), rating_(rating) {}

void Track::loadMetadata() {
    Mp3Metadata metadata = Mp3Metadata::fromFile(QString::fromStdString(path_));

    if (!metadata.title.isEmpty()) title_ = metadata.title.toStdString();
    if (!metadata.artist.isEmpty()) artist_ = metadata.artist.toStdString();
    if (!metadata.album.isEmpty()) album_ = metadata.album.toStdString();
    if (!metadata.genre.isEmpty()) genre_ = metadata.genre.toStdString();
    if (!metadata.year.isEmpty()) year_ = metadata.year.toStdString();
    if (!metadata.coverImage.isNull()) {
        coverImage_ = metadata.coverImage;
        coverLoaded_ = true;
    }
}

void Track::loadMetadataFast() {
    QFileInfo fileInfo(QString::fromStdString(path_));
    QString baseName = fileInfo.baseName();
    baseName.remove(".mp3", Qt::CaseInsensitive);

    QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

    if (parts.size() >= 2) {
        artist_ = parts[0].trimmed().toStdString();
        QString title = parts[1].trimmed();

        while (!title.isEmpty() && title[0].isDigit()) title.remove(0, 1);
        if (!title.isEmpty() && (title[0] == '_' || title[0] == '.' || title[0] == '-' || title[0] == ' ')) {
            title.remove(0, 1);
        }
        title_ = title.toStdString();
    } else {
        QString title = baseName.trimmed();
        while (!title.isEmpty() && title[0].isDigit()) title.remove(0, 1);
        while (!title.isEmpty() && (title[0] == '_' || title[0] == '.' || title[0] == '-' || title[0] == ' ')) {
            title.remove(0, 1);
        }
        title_ = title.toStdString();
        artist_ = "Unknown Artist";
    }

    album_ = "Unknown Album";
    genre_ = "Unknown";
    year_ = "Unknown";
}

void Track::loadMetadataAsync() {
    TrackMetadata metadata = FastMetadataReader::instance().getMetadata(
        QString::fromStdString(path_));

    if (!metadata.title.isEmpty()) title_ = metadata.title.toStdString();
    if (!metadata.artist.isEmpty()) artist_ = metadata.artist.toStdString();
    if (!metadata.album.isEmpty()) album_ = metadata.album.toStdString();
    if (!metadata.genre.isEmpty()) genre_ = metadata.genre.toStdString();
    if (!metadata.year.isEmpty()) year_ = metadata.year.toStdString();
    if (!metadata.coverImage.isNull()) {
        coverImage_ = metadata.coverImage;
        coverLoaded_ = true;
    }
}

QImage Track::getCoverImage() const {
    if (coverLoaded_ && !coverImage_.isNull()) return coverImage_;

    QImage cover = extractCoverFromMP3();
    if (!cover.isNull()) return cover;

    return loadDefaultCover();
}

QImage Track::extractCoverFromMP3() const {
    return Mp3Metadata::extractCover(QString::fromStdString(path_));
}

QImage Track::loadDefaultCover() const {
    QString coverPath = ResourceFinder::findDefaultCover();
    if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
        QImage image(coverPath);
        if (!image.isNull()) return image;
    }

    QImage grayImage(200, 200, QImage::Format_RGB32);
    grayImage.fill(Qt::darkGray);
    return grayImage;
}
