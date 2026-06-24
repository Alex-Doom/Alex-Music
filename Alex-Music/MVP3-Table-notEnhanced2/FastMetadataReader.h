#pragma once
#include <QString>
#include <QImage>
#include <QHash>
#include <QMutex>
#include <QThread>
#include <QDateTime>
#include <atomic>

struct TrackMetadata {
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    int trackNumber = 0;
    int duration = 0;
    QImage coverImage;
    bool isValid = false;
    qint64 lastAccess = 0;
};

class FastMetadataReader : public QObject {
    Q_OBJECT
public:
    static FastMetadataReader& instance();

    TrackMetadata getMetadata(const QString& filePath);
    void loadBatchAsync(const QStringList& files);
    void clearCache();
    int cacheSize() const { return cache_.size(); }
    void preloadMetadata(const QStringList& files);

signals:
    void metadataLoaded(const QString& filePath, const TrackMetadata& metadata);
    void batchProgress(int current, int total);
    void batchFinished();

private:
    FastMetadataReader();
    ~FastMetadataReader();

    TrackMetadata readMetadataFast(const QString& filePath);
    TrackMetadata readFromFileName(const QString& filePath);
    TrackMetadata quickParseID3v2(const QString& filePath);
    bool hasID3v2Tag(const QString& filePath);
    void pruneCache();

    QHash<QString, TrackMetadata> cache_;
    mutable QMutex cacheMutex_;
    QThread* workerThread_;
    std::atomic<bool> isRunning_;

    static const int MAX_CACHE_SIZE = 1000;
    static const qint64 CACHE_TTL = 3600000; // 1 час
};
