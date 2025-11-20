#pragma once
#include <string>
// #include <optional>
#include <QImage>

class Track {
public:
    Track() {};
    Track(std::string path, std::string artist, std::string title,
          std::string album = "", double rating = 0.0);

    const std::string& path() const { return path_; }
    const std::string& artist() const { return artist_; }
    const std::string& title() const { return title_; }
    const std::string& album() const { return album_; }
    double rating() const { return rating_; }

    void setTrackRating(double rating) { rating_ = rating; }

    // ОСНОВНОЙ МЕТОД: получает обложку (либо из MP3, либо default.jpg)
    QImage getCoverImage() const;

private:
    std::string path_;
    std::string artist_;
    std::string title_;
    std::string album_;
    double rating_;

    // МЕТОД 1: извлечение обложки из MP3 файла
    QImage extractCoverFromMP3() const;

    // МЕТОД 2: загрузка обложки по умолчанию
    QImage loadDefaultCover() const;
};
