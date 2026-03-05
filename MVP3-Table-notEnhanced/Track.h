#pragma once
#include <string>
#include <QImage>
#include <QString>

class Track {
public:
    Track() {}; // конструктор по умолчанию
    Track(std::string path, std::string artist = "", std::string title = "",
          std::string album = "", std::string genre = "", std::string year = "",
          double rating = 0.0); // конструктор с параметрами

    // Геттеры для доступа к приватным полям (возвращают константные ссылки)
    const std::string& path() const { return path_; }
    const std::string& artist() const { return artist_; }
    const std::string& title() const { return title_; }
    const std::string& album() const { return album_; }
    const std::string& genre() const { return genre_; }
    const std::string& year() const { return year_; }
    double rating() const { return rating_; }

    // Геттеры в QString для удобства Qt
    QString qTitle() const { return QString::fromStdString(title_); }
    QString qArtist() const { return QString::fromStdString(artist_); }
    QString qAlbum() const { return QString::fromStdString(album_); }
    QString qGenre() const { return QString::fromStdString(genre_); }
    QString qYear() const { return QString::fromStdString(year_); }

    // Сеттеры
    void setTrackRating(double rating) { rating_ = rating; }
    void setTitle(const std::string& title) { title_ = title; }
    void setArtist(const std::string& artist) { artist_ = artist; }
    void setAlbum(const std::string& album) { album_ = album; }
    void setGenre(const std::string& genre) { genre_ = genre; }
    void setYear(const std::string& year) { year_ = year; }

    std::string getID() const;
    QImage getCoverImage() const;

    // Загрузка метаданных из файла
    void loadMetadata();

private:
    // Приватные поля класса
    std::string path_;     // Путь к файлу трека
    std::string artist_;   // Исполнитель
    std::string title_;    // Название трека
    std::string album_;    // Альбом
    std::string genre_;
    std::string year_;
    double rating_;        // Рейтинг от 0.0 до 5.0

    // МЕТОД 1: извлечение обложки из MP3 файла
    QImage extractCoverFromMP3() const;

    // МЕТОД 2: загрузка обложки по умолчанию (картинка "default.jpg")
    QImage loadDefaultCover() const;
};
