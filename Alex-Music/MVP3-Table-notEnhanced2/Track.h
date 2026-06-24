#pragma once
#include <string>
#include <QImage>
#include <QString>

class Track {
public:
    Track() = default;
    explicit Track(const std::string& path);
    Track(std::string path, std::string artist, std::string title,
          std::string album = "", double rating = 0.0);

    // Геттеры
    const std::string& path() const { return path_; }
    const std::string& artist() const { return artist_; }
    const std::string& title() const { return title_; }
    const std::string& album() const { return album_; }
    const std::string& genre() const { return genre_; }
    const std::string& year() const { return year_; }
    double rating() const { return rating_; }

    // Qt геттеры
    QString qPath() const { return QString::fromStdString(path_); }
    QString qArtist() const { return QString::fromStdString(artist_); }
    QString qTitle() const { return QString::fromStdString(title_); }
    QString qAlbum() const { return QString::fromStdString(album_); }
    QString qGenre() const { return QString::fromStdString(genre_); }
    QString qYear() const { return QString::fromStdString(year_); }

    // Сеттеры
    void setArtist(const std::string& artist) { artist_ = artist; }
    void setTitle(const std::string& title) { title_ = title; }
    void setAlbum(const std::string& album) { album_ = album; }
    void setGenre(const std::string& genre) { genre_ = genre; }
    void setYear(const std::string& year) { year_ = year; }
    void setTrackRating(double rating) { rating_ = rating; }

    // Метаданные
    void loadMetadata();          // Полная загрузка через QMediaPlayer
    void loadMetadataFast();      // Быстрая загрузка из имени файла
    void loadMetadataAsync();     // Асинхронная загрузка через FastMetadataReader

    // Обложка
    QImage getCoverImage() const;

    // ID
    std::string getID() const { return path_; }

private:
    std::string path_;
    std::string artist_ = "Unknown Artist";
    std::string title_ = "Unknown Title";
    std::string album_ = "Unknown Album";
    std::string genre_ = "Unknown";
    std::string year_ = "Unknown";
    double rating_ = 0.0;

    mutable QImage coverImage_;
    mutable bool coverLoaded_ = false;

    QImage extractCoverFromMP3() const;
    QImage loadDefaultCover() const;
};
