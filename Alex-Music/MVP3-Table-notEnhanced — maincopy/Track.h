#pragma once
#include <QString>
#include <QImage>
#include <atomic>

class Track {
public:
    Track() = default;
    explicit Track(const QString& filePath);

    // Геттеры
    QString path() const { return path_; }
    QString artist() const { return artist_; }
    QString title() const { return title_; }
    QString album() const { return album_; }
    QString genre() const { return genre_; }
    QString year() const { return year_; }
    double rating() const { return rating_.load(); }
    int duration() const { return duration_; }

    // Сеттеры
    void setArtist(const QString& artist) { artist_ = artist; }
    void setTitle(const QString& title) { title_ = title; }
    void setAlbum(const QString& album) { album_ = album; }
    void setGenre(const QString& genre) { genre_ = genre; }
    void setYear(const QString& year) { year_ = year; }
    void setRating(double rating) { rating_.store(rating); }
    void setDuration(int seconds) { duration_ = seconds; }

    // Метаданные
    void parseFromFilename();  // Быстрый парсинг
    void loadFullMetadata();   // Полная загрузка через Qt

    // Обложка
    QImage cover() const;
    void setCover(const QImage& cover) { cover_ = cover; coverLoaded_ = true; }

    // Уникальный ID для сохранения рейтингов
    QString uniqueId() const { return path_; }

private:
    QString path_;
    QString artist_ = "Unknown Artist";
    QString title_ = "Unknown Title";
    QString album_ = "Unknown Album";
    QString genre_ = "Unknown";
    QString year_ = "Unknown";
    std::atomic<double> rating_{0.0};
    int duration_ = 0;

    mutable QImage cover_;
    mutable std::atomic<bool> coverLoaded_{false};
};
