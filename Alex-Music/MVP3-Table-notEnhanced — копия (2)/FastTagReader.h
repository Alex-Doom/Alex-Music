// FastTagReader.h
#pragma once

#include <QString>
#include <QImage>
#include <QObject>
#include <QDebug>

// Предварительные объявления для taglib
namespace TagLib {
class String;
class MPEG;
namespace ID3v2 {
class Tag;
class AttachedPictureFrame;
class PopularimeterFrame;
}
}

// Импортируем структуру TrackMetadata
#include "FastMetadataReader.h"

// Класс для быстрого чтения метаданных с использованием taglib
class FastTagReader {
public:
    // Чтение метаданных из файла
    static TrackMetadata readMetadata(const QString& filePath);

    // Сохранение рейтинга в файл
    static bool saveRating(const QString& filePath, double rating);

    // Сохранение обложки в файл (опционально)
    static bool saveCover(const QString& filePath, const QImage& cover);

    // Проверка, является ли файл валидным MP3
    static bool isValidMp3(const QString& filePath);

    // Получение длительности без загрузки полных метаданных
    static int getDuration(const QString& filePath);

private:
    // Вспомогательные методы
    static QString decodeTaglibString(const TagLib::String& str);
    static QImage extractCoverImage(TagLib::ID3v2::Tag* tag);
};

// Класс-обертка для использования в существующей архитектуре
class FastTagReaderWrapper : public QObject {
    Q_OBJECT
public:
    explicit FastTagReaderWrapper(QObject* parent = nullptr);
    ~FastTagReaderWrapper();

    // Асинхронное чтение метаданных
    void readMetadataAsync(const QString& filePath);

    // Пакетное чтение
    void readBatchAsync(const QStringList& files);

    // Отмена текущей загрузки
    void cancel();

signals:
    void metadataReady(const QString& filePath, const TrackMetadata& metadata);
    void batchProgress(int current, int total);
    void batchFinished();

private:
    void doReadMetadata(const QString& filePath);
    void doBatchRead(const QStringList& files);

    std::atomic<bool> isCancelled_{false};
};
