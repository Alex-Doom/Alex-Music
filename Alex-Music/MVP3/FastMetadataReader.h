// FastMetadataReader.h
#pragma once
#include <QString>
#include <QImage>
#include <QHash>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <QFile>

// Структура для хранения метаданных
struct TrackMetadata {
    QString title;           // Название трека
    QString artist;          // Исполнитель
    QString album;           // Альбом
    QString genre;           // Жанр
    QString year;            // Год
    int trackNumber = 0;     // Номер трека в альбоме
    int duration = 0;        // Длительность в секундах
    QImage coverImage;       // Обложка альбома
    bool isValid = false;    // Флаг валидности
    qint64 lastAccess = 0;   // Время последнего доступа (для LRU кэша)
};

// Класс для быстрого чтения метаданных с кэшированием
class FastMetadataReader : public QObject {
    Q_OBJECT
public:
    // Singleton для единого кэша
    static FastMetadataReader& instance();

    // Быстрое получение метаданных (с кэшем)
    TrackMetadata getMetadata(const QString& filePath);

    // Асинхронная загрузка множества файлов
    void loadBatchAsync(const QStringList& files);

    // Очистка кэша
    void clearCache();

    // Получение статистики кэша
    int cacheSize() const { return cache_.size(); }

    // Предварительная загрузка (для быстрого старта)
    void preloadMetadata(const QStringList& files);

signals:
    void metadataLoaded(const QString& filePath, const TrackMetadata& metadata);
    void batchProgress(int current, int total);
    void batchFinished();

private:
    FastMetadataReader();
    ~FastMetadataReader();

    // Основной метод чтения метаданных
    TrackMetadata readMetadataFast(const QString& filePath);

    // Метод чтения из имени файла (быстрый fallback)
    TrackMetadata readFromFileName(const QString& filePath);

    // Быстрый парсинг ID3v2 заголовка
    TrackMetadata quickParseID3v2(const QString& filePath);

    // Проверка наличия ID3v2 тега без полного парсинга
    bool hasID3v2Tag(const QString& filePath);

    // Кэш метаданных
    QHash<QString, TrackMetadata> cache_;
    mutable QMutex cacheMutex_;

    // Поток для фоновой загрузки
    QThread* workerThread_;
    std::atomic<bool> isRunning_;

    // Максимальный размер кэша (1000 записей)
    static const int MAX_CACHE_SIZE = 1000;

    // LRU кэш - удаление старых записей
    void pruneCache();

    // Время жизни кэша в миллисекундах (1 час)
    static const qint64 CACHE_TTL = 3600000;
};
