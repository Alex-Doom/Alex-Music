#pragma once
#include <string>
#include <QImage>

class Track {
public:
    Track() {}; // конструктор по умолчанию
    Track(std::string path, std::string artist, std::string title,
          std::string album = "", double rating = 0.0); // конструктор с параметрами

    // Геттеры для доступа к приватным полям (возвращают константные ссылки)
    const std::string& path() const { return path_; }
    const std::string& artist() const { return artist_; }
    const std::string& title() const { return title_; }
    const std::string& album() const { return album_; }
    double rating() const { return rating_; }

    // Сеттер для установки рейтинга трека
    void setTrackRating(double rating) { rating_ = rating; }

    // Метод для получения уникального идентификатора трека (путь к файлу)
    std::string getID() const;

    // метод, получает обложку (либо из MP3, либо default.jpg)
    QImage getCoverImage() const;

    // Геттеры для дополнительных метаданных
    const std::string& genre() const { return genre_; }
    const std::string& year() const { return year_; }
    int trackNumber() const { return trackNumber_; }

    // Вывод всех метаданных в терминал
    void printMetadata() const;

private:
    // Приватные поля класса
    std::string path_;     // Путь к файлу трека
    std::string artist_;   // Исполнитель
    std::string title_;    // Название трека
    std::string album_;    // Альбом
    double rating_;        // Рейтинг от 0.0 до 5.0

    std::string genre_;
    std::string year_;
    int trackNumber_ = 0;
    std::string publisher_;  // издатель
    std::string barcode_;    // штрих-код

    // МЕТОД 1: извлечение обложки из MP3 файла
    QImage extractCoverFromMP3() const;

    // МЕТОД 2: загрузка обложки по умолчанию (картинка "default.jpg")
    QImage loadDefaultCover() const;

    bool loadMetadataFromMP3();
};
