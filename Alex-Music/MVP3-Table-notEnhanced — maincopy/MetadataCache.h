#pragma once
#include "Track.h"
#include <QHash>
#include <QMutex>
#include <QThreadPool>
#include <QRunnable>
#include <QCache>

struct MetadataResult {
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    int duration = 0;
    QImage cover;
    bool valid = false;
};

class MetadataCache {
public:
    static MetadataCache& instance();

    MetadataResult get(const QString& path);
    void prefetch(const QStringList& paths);
    void clear() { cache_.clear(); }

private:
    MetadataCache() = default;
    QCache<QString, MetadataResult> cache_{1000}; // LRU на 1000 элементов
    QMutex mutex_;

    MetadataResult readFromFile(const QString& path);
};
