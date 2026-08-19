// FastMetadataReader.cpp
#include "FastMetadataReader.h"
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDataStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#include <QtCore5Compat/QTextCodec>   // ← ДЛЯ QT6 с Core5Compat
#else
#include <QTextCodec>
#endif

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

// Проверка наличия ID3v2 тега без загрузки всего файла
bool FastMetadataReader::hasID3v2Tag(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Читаем только первые 10 байт для проверки ID3v2
    QByteArray header = file.read(10);
    file.close();

    // ID3v2 тег начинается с "ID3"
    return (header.size() >= 10 && header.startsWith("ID3"));
}

// Быстрый парсинг ID3v2 заголовка (только основные поля)
TrackMetadata FastMetadataReader::quickParseID3v2(const QString& filePath) {
    TrackMetadata metadata;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return metadata;
    }

    QByteArray data = file.read(4096); // Читаем только первые 4KB
    file.close();

    if (data.size() < 10) {
        return metadata;
    }

    // Проверяем ID3v2 заголовок
    if (!data.startsWith("ID3")) {
        return metadata;
    }

    // Получаем размер тега (синхронизированный)
    int tagSize = ((data[6] & 0x7F) << 21) |
                  ((data[7] & 0x7F) << 14) |
                  ((data[8] & 0x7F) << 7) |
                  (data[9] & 0x7F);

    // Ищем TIT2 (название), TPE1 (исполнитель), TALB (альбом)
    int pos = 10;
    while (pos < data.size() - 10 && pos < tagSize + 10) {
        // Проверяем, что это фрейм
        if (data[pos] == 0) break;

        // Читаем ID фрейма (4 байта)
        QByteArray frameId = data.mid(pos, 4);
        if (frameId.size() < 4) break;

        // Размер фрейма (4 байта)
        quint32 frameSize = (static_cast<quint8>(data[pos + 4]) << 24) |
                            (static_cast<quint8>(data[pos + 5]) << 16) |
                            (static_cast<quint8>(data[pos + 6]) << 8) |
                            static_cast<quint8>(data[pos + 7]);

        // Пропускаем флаги (2 байта)
        pos += 10;

        if (pos + frameSize > data.size()) {
            break;
        }

        // Читаем данные фрейма
        if (frameId == "TIT2") { // Название
            // Пропускаем кодировку (1 байт)
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.title = decodeId3Text(textData).trimmed();
        }
        else if (frameId == "TPE1") { // Исполнитель
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.artist = decodeId3Text(textData).trimmed();
        }
        else if (frameId == "TALB") { // Альбом
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.album = decodeId3Text(textData).trimmed();
        }
        else if (frameId == "TCON") { // Жанр
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.genre = decodeId3Text(textData).trimmed();
        }
        else if (frameId == "TYER" || frameId == "TDRC") { // Год
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.year = decodeId3Text(textData).trimmed();
            if (metadata.year.length() > 4) {
                metadata.year = metadata.year.left(4);
            }
        }
        else if (frameId == "TRCK") { // Номер трека
            int textStart = pos + 1;
            QByteArray textData = data.mid(textStart, frameSize - 1);
            metadata.trackNumber = decodeId3Text(textData).split("/")[0].toInt();
        }

        pos += frameSize;
    }

    metadata.isValid = !metadata.title.isEmpty() || !metadata.artist.isEmpty();
    return metadata;
}

// Чтение метаданных из имени файла (очень быстро)
TrackMetadata FastMetadataReader::readFromFileName(const QString& filePath) {
    TrackMetadata metadata;
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();

    // Удаляем расширение .mp3 если осталось
    baseName.remove(".mp3", Qt::CaseInsensitive);

    qDebug() << "Парсинг имени файла:" << baseName;

    // Формат: "Artist - Title.mp3" или "Title.mp3"
    QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

    if (parts.size() >= 2) {
        metadata.artist = parts[0].trimmed();
        metadata.title = parts[1].trimmed();

        // Очищаем название от цифр в начале
        while (!metadata.title.isEmpty() && metadata.title[0].isDigit()) {
            metadata.title.remove(0, 1);
        }
        // Удаляем только первый символ если это подчеркивание, точка или дефис
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

        // Очищаем название от цифр в начале
        while (!metadata.title.isEmpty() && metadata.title[0].isDigit()) {
            metadata.title.remove(0, 1);
        }
        // Удаляем подчеркивания, точки, дефисы в начале
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

// Основной быстрый метод получения метаданных
TrackMetadata FastMetadataReader::getMetadata(const QString& filePath) {
    // 1. Проверяем кэш
    {
        QMutexLocker locker(&cacheMutex_);
        if (cache_.contains(filePath)) {
            TrackMetadata cached = cache_[filePath];
            cached.lastAccess = QDateTime::currentMSecsSinceEpoch();
            cache_[filePath] = cached;
            return cached;
        }
    }

    // 2. Быстрая проверка наличия ID3v2 тега
    bool hasID3 = hasID3v2Tag(filePath);

    TrackMetadata metadata;

    // 3. Если есть ID3v2, пробуем быстро распарсить
    if (hasID3) {
        metadata = quickParseID3v2(filePath);
    }

    // 4. Если не удалось распарсить ID3v2 или нет тегов, используем имя файла
    if (!metadata.isValid || metadata.title.isEmpty()) {
        metadata = readFromFileName(filePath);
    }

    metadata.lastAccess = QDateTime::currentMSecsSinceEpoch();

    // 5. Сохраняем в кэш
    {
        QMutexLocker locker(&cacheMutex_);
        cache_[filePath] = metadata;
        pruneCache(); // Удаляем старые записи
    }

    return metadata;
}

// LRU кэш - удаляем старые записи
void FastMetadataReader::pruneCache() {
    if (cache_.size() <= MAX_CACHE_SIZE) {
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QList<QString> toRemove;

    // Находим записи старше CACHE_TTL
    for (auto it = cache_.begin(); it != cache_.end(); ++it) {
        if (now - it.value().lastAccess > CACHE_TTL) {
            toRemove.append(it.key());
        }
    }

    // Удаляем старые записи
    for (const QString& key : toRemove) {
        cache_.remove(key);
    }

    // Если всё ещё слишком много, удаляем самые старые
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

// Асинхронная пакетная загрузка
void FastMetadataReader::loadBatchAsync(const QStringList& files) {
    QThread* worker = new QThread;
    QObject* workerObj = new QObject;
    workerObj->moveToThread(worker);

    // ИСПРАВЛЕНИЕ: добавляем worker в capture list
    connect(worker, &QThread::started, [this, files, workerObj, worker]() {
        int total = files.size();
        for (int i = 0; i < total && !workerObj->thread()->isInterruptionRequested(); ++i) {
            const QString& file = files[i];

            // Проверяем кэш
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

            // Даем другим потокам время
            if (i % 100 == 0) {
                QThread::msleep(1);
            }
        }
        emit batchFinished();
        workerObj->deleteLater();
        worker->quit();  // worker теперь захвачен
    });

    connect(worker, &QThread::finished, worker, &QThread::deleteLater);
    worker->start();
}

// Предварительная загрузка (для быстрого старта)
void FastMetadataReader::preloadMetadata(const QStringList& files) {
    // Загружаем только первые 100 файлов для быстрого отображения
    int preloadCount = qMin(100, files.size());
    for (int i = 0; i < preloadCount; ++i) {
        getMetadata(files[i]);
    }
}

void FastMetadataReader::clearCache() {
    QMutexLocker locker(&cacheMutex_);
    cache_.clear();
}

// Вспомогательная функция: подсчёт спецсимволов Latin-1 Supplement (U+00C0-U+00FF)
static int countLatin1Supplement(const QString& text) {
    int count = 0;
    for (const QChar& c : text) {
        ushort u = c.unicode();
        if (u >= 0x00C0 && u <= 0x00FF) {
            count++;
        }
    }
    return count;
}

QString decodeId3Text(const QByteArray& data) {
    if (data.isEmpty()) return QString();

    QString result = QString::fromUtf8(data);

    // === ВАША ЛОГИКА ===
    int specialChars = countLatin1Supplement(result);
    if (specialChars <= 1) {
        return result; // 0 или 1 — оставляем
    }

    // >1 — пробуем CP1251
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder("Windows-1251");
    QString fixed = decoder.decode(data);
#else
    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    QString fixed = codec->toUnicode(data);
#endif

    for (const QChar& c : fixed) {
        if (c.unicode() >= 0x0400 && c.unicode() <= 0x04FF) {
            return fixed;
        }
    }

    return result;
}
