#pragma once
#include <string>
#include <QImage>
#include <QMediaPlayer>
#include <QEventLoop>
#include <QTimer>

class Track {
public:
    Track() {}; // конструктор по умолчанию
    Track(std::string path, std::string artist, std::string title,
          std::string album = "", double rating = 0.0,
          int year = 0, std::string genre = ""); // конструктор с параметрами

    // Геттеры для доступа к приватным полям (возвращают константные ссылки)
    const std::string& path() const { return path_; }
    const std::string& artist() const { return artist_; }
    const std::string& title() const { return title_; }
    const std::string& album() const { return album_; }
    double rating() const { return rating_; }
    int year() const { return year_; }
    const std::string& genre() const { return genre_; }

    // Сеттер для установки рейтинга трека
    void setTrackRating(double rating) { rating_ = rating; }
    void setYear(int year) { year_ = year; }
    void setGenre(const std::string& genre) { genre_ = genre; }
    void setAlbum(const std::string& album) { album_ = album; }

    // Метод для получения уникального идентификатора трека (путь к файлу)
    std::string getID() const;

    // метод, получает обложку (либо из MP3, либо default.jpg)
    QImage getCoverImage() const;

    void loadMetadata();  // Новый метод для загрузки метаданных

private:
    // Приватные поля класса
    std::string path_;     // Путь к файлу трека
    std::string artist_;   // Исполнитель
    std::string title_;    // Название трека
    std::string album_;    // Альбом
    double rating_;        // Рейтинг от 0.0 до 5.0
    int year_;
    std::string genre_;

    // МЕТОД 1: извлечение обложки из MP3 файла
    QImage extractCoverFromMP3() const;

    // МЕТОД 2: загрузка обложки по умолчанию (картинка "default.jpg")
    QImage loadDefaultCover() const;
};
