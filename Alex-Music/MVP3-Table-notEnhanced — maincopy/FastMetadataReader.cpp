// FastMetadataReader.cpp
#include "FastMetadataReader.h"
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDataStream>
#include <QThread>
#include <QDateTime>

FastMetadataReader::FastMetadataReader() : isRunning_(false) {
    workerThread_ = new QThread(this);
    workerThread_->start();
}

FastMetadataReader::~FastMetadataReader() {
    isRunning_ = false;
    workerThread_->quit();
    workerThread_->wait();
}

FastMetadataReader& FastMetadataReader::instance() {
    static FastMetadataReader reader;
    return reader;
}

bool FastMetadataReader::hasID3v2Tag(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray header = file.read(10);
    file.close();

    return (header.size() >= 10 && header.startsWith("ID3"));
}

TrackMetadata FastMetadataReader::quickParseID3v2(const QString& filePath) {
    TrackMetadata metadata;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return metadata;
    }

    QByteArray data = file.read(4096);
    file.close();

    if (data.size() < 10 || !data.startsWith("ID3")) {
        return metadata;
    }

    int tagSize = ((data[6] & 0x7F) << 21) |
                  ((data[7] & 0x7F) << 14) |
                  ((data[8] & 0x7F) << 7) |
                  (data[9] & 0x7F);

    int pos = 10;
    while (pos < data.size() - 10 && pos < tagSize + 10) {
        if (data[pos] == 0) break;

        QByteArray frameId = data.mid(pos, 4);
        if (frameId.size() < 4) break;

        quint32 frameSize = (static_cast<quint8>(data[pos + 4]) << 24) |
                            (static_cast<quint8>(data[pos + 5]) << 16) |
                            (static_cast<quint8>(data[pos + 6]) << 8) |
                            static_cast<quint8>(data[pos + 7]);

        pos += 10;

        if (pos + frameSize > data.size()) {
            break;
        }

        if (frameId == "TIT2") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.title = QString::fromUtf8(textData).trimmed();
        }
        else if (frameId == "TPE1") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.artist = QString::fromUtf8(textData).trimmed();
        }
        else if (frameId == "TALB") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.album = QString::fromUtf8(textData).trimmed();
        }
        else if (frameId == "TCON") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.genre = QString::fromUtf8(textData).trimmed();
        }
        else if (frameId == "TYER" || frameId == "TDRC") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.year = QString::fromUtf8(textData).trimmed();
            if (metadata.year.length() > 4) {
                metadata.year = metadata.year.left(4);
            }
        }
        else if (frameId == "TRCK") {
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.trackNumber = QString::fromUtf8(textData).split("/")[0].toInt();
        }

        pos += frameSize;
    }

    metadata.isValid = !metadata.title.isEmpty() || !metadata.artist.isEmpty();
    return metadata;
}

TrackMetadata FastMetadataReader::readFromFileName(const QString& filePath) {
    TrackMetadata metadata;
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();

    baseName.remove(".mp3", Qt::CaseInsensitive);

    qDebug() << "Парсинг имени файла:" << baseName;

    QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

    if (parts.size() >= 2) {
        metadata.artist = parts[0].trimmed();
        metadata.title = parts[1].trimmed();

        while (!metadata.title.isEmpty() && metadata.title[0].isDigit()) {
            metadata.title.remove(0, 1);
        }
        if (!metadata.title.isEmpty() && (metadata.title[0] == '_' ||
                                          metadata.title[0] == '.' ||
                                          metadata.title[0] == '-' ||
                                          metadata.title[0] == ' ')) {
            metadata.title.remove(0, 1);
        }

        qDebug() << "  Распознано:" << metadata.artist << "-" << metadata.title;
    } else {
        metadata.title = baseName.trimmed();
        metadata.artist = "Unknown Artist";

        while (!metadata.title.isEmpty() && metadata.title[0].isDigit()) {
            metadata.title.remove(0, 1);
        }
        while (!metadata.title.isEmpty() && (metadata.title[0] == '_' ||
                                             metadata.title[0] == '.' ||
                                             metadata.title[0] == '-' ||
                                             metadata.title[0] == ' ')) {
            metadata.title.remove(0, 1);
        }

        qDebug() << "  Только название:" << metadata.title;
    }

    metadata.album = "Unknown Album";
    metadata.genre = "Unknown";
    metadata.year = "Unknown";
    metadata.isValid = true;

    return metadata;
}

TrackMetadata FastMetadataReader::readMetadataFast(const QString& filePath) {
    TrackMetadata metadata;
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || fileInfo.size() == 0) {
        return readFromFileName(filePath);
    }

    bool hasID3 = hasID3v2Tag(filePath);

    if (hasID3) {
        metadata = quickParseID3v2(filePath);
    }

    if (!metadata.isValid || metadata.title.isEmpty()) {
        metadata = readFromFileName(filePath);
    }

    metadata.lastAccess = QDateTime::currentDateTime().toMSecsSinceEpoch();

    return metadata;
}

TrackMetadata FastMetadataReader::getMetadata(const QString& filePath) {
    {
        QMutexLocker locker(&cacheMutex_);
        if (cache_.contains(filePath)) {
            TrackMetadata cached = cache_[filePath];
            cached.lastAccess = QDateTime::currentDateTime().toMSecsSinceEpoch();
            cache_[filePath] = cached;
            return cached;
        }
    }

    TrackMetadata metadata = readMetadataFast(filePath);
    metadata.lastAccess = QDateTime::currentDateTime().toMSecsSinceEpoch();

    {
        QMutexLocker locker(&cacheMutex_);
        cache_[filePath] = metadata;
        pruneCache();
    }

    return metadata;
}

void FastMetadataReader::pruneCache() {
    if (cache_.size() <= MAX_CACHE_SIZE) {
        return;
    }

    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QList<QString> toRemove;

    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (now - it.value().lastAccess > CACHE_TTL) {
            toRemove.append(it.key());
        }
    }

    for (const QString& key : toRemove) {
        cache_.remove(key);
    }

    if (cache_.size() > MAX_CACHE_SIZE) {
        QList<QPair<qint64, QString>> ages;
        for (auto it = cache_.begin(); it != cache_.end(); ++it) {
            ages.append(qMakePair(it.value().lastAccess, it.key()));
        }
        std::sort(ages.begin(), ages.end());

        int toDelete = cache_.size() - MAX_CACHE_SIZE;
        for (int i = 0; i < toDelete; ++i) {
            cache_.remove(ages[i].second);
        }
    }
}

void FastMetadataReader::loadBatchAsync(const QStringList& files) {
    QThread* worker = new QThread;
    QObject* workerObj = new QObject;
    workerObj->moveToThread(worker);

    connect(worker, &QThread::started, [this, files, workerObj, worker]() {
        int total = files.size();
        for (int i = 0; i < total && !workerObj->thread()->isInterruptionRequested(); ++i) {
            const QString& file = files[i];

            bool inCache = false;
            {
                QMutexLocker locker(&cacheMutex_);
                inCache = cache_.contains(file);
            }

            if (!inCache) {
                TrackMetadata metadata = getMetadata(file);
                emit metadataLoaded(file, metadata);
            }

            emit batchProgress(i + 1, total);

            if (i % 100 == 0) {
                QThread::msleep(1);
            }
        }
        emit batchFinished();
        workerObj->deleteLater();
        worker->quit();
        worker->deleteLater();
    });

    connect(worker, &QThread::finished, worker, &QThread::deleteLater);
    worker->start();
}

void FastMetadataReader::preloadMetadata(const QStringList& files) {
    qDebug() << "Предварительная загрузка метаданных для" << files.size() << "файлов";

    for (const QString& file : files) {
        getMetadata(file);
        QThread::msleep(1);
    }

    qDebug() << "Предварительная загрузка завершена";
}

void FastMetadataReader::clearCache() {
    QMutexLocker locker(&cacheMutex_);
    cache_.clear();
}
