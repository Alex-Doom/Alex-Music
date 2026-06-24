// Track.h
#pragma once
#include <string>
#include <QImage>
#include <QString>
#include <QUuid>

class Track {
public:
    // Конструкторы
    Track();
    explicit Track(const std::string& filePath);
    Track(const std::string& filePath, const std::string& artist,
          const std::string& title, const std::string& album = "",
          double rating = 0.0);

    // Уникальный идентификатор (не зависит от индекса)
    QString id() const { return id_; }

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
    void setRating(double rating) { rating_ = rating; }

    // Метаданные
    bool hasMetadata() const { return hasMetadata_; }
    void setHasMetadata(bool has) { hasMetadata_ = has; }

    // Обложка
    QImage coverImage() const;
    void setCoverImage(const QImage& image);
    bool hasCoverImage() const { return !coverImage_.isNull(); }

    // Загрузка метаданных
    void loadMetadataFast();      // Быстрая загрузка из имени файла
    void loadMetadataFull();      // Полная загрузка из файла

    // Операторы сравнения
    bool operator==(const Track& other) const { return id_ == other.id_; }
    bool operator!=(const Track& other) const { return id_ != other.id_; }

private:
    QString id_;                   // Уникальный идентификатор
    std::string path_;             // Путь к файлу
    std::string artist_;           // Исполнитель
    std::string title_;            // Название трека
    std::string album_;            // Альбом
    std::string genre_;            // Жанр
    std::string year_;             // Год
    double rating_ = 0.0;          // Рейтинг
    bool hasMetadata_ = false;     // Флаг наличия метаданных
    QImage coverImage_;            // Обложка
    bool coverLoaded_ = false;     // Флаг загрузки обложки

    void generateId();             // Генерация уникального ID
    QImage extractCoverFromMP3() const;
    QImage loadDefaultCover() const;
};
