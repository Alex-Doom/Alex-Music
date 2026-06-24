// Track.h
#pragma once
#include <string>
#include <QImage>
#include <QString>

class Track {
public:
    Track() {}
    Track(const std::string& path);
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

    // Геттеры для Qt
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

    // Загрузка метаданных
    void loadMetadata();

    // Обложка
    QImage getCoverImage() const;

    std::string getID() const;


    // Быстрая загрузка метаданных (синхронно, только из имени файла)
    void loadMetadataFast();

    // Асинхронная загрузка реальных метаданных
    void loadMetadataAsync();
private:
    std::string path_;
    std::string artist_;
    std::string title_;
    std::string album_;
    std::string genre_;
    std::string year_;
    double rating_ = 0.0;

    QImage coverImage_;
    bool coverLoaded_ = false;

    QImage extractCoverFromMP3() const;
    QImage loadDefaultCover() const;

    bool metadataLoaded_ = false;
};
