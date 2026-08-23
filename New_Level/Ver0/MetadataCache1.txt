#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QMutex>

struct CachedTrack {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    int duration = 0;       // секунды
    qint64 fileSize = 0;    // байты
    double rating = 0.0;
    qint64 fileModTime = 0; // время модификации файла (для инвалидации кэша)
    bool isValid = true;    // false = битый трек
};

class MetadataCache {
public:
    static MetadataCache& instance();

    // Инициализация (вызывать один раз при старте)
    bool initialize(const QString& dbPath);
    void close();

    // === ОСНОВНЫЕ ОПЕРАЦИИ ===

    // Загрузить ВСЕ треки для папки (мгновенно)
    QVector<CachedTrack> loadFolder(const QString& folderPath);

    // Загрузить один трек
    bool loadTrack(const QString& filePath, CachedTrack& track);

    // Сохранить/обновить трек в кэше
    void saveTrack(const CachedTrack& track);

    // Пакетное сохранение (для фоновой загрузки)
    void saveTracks(const QVector<CachedTrack>& tracks);

    // Удалить трек из кэша
    void removeTrack(const QString& filePath);

    // Очистить кэш для папки
    void clearFolder(const QString& folderPath);

    // Проверить, актуален ли кэш для файла
    bool isCacheValid(const QString& filePath, qint64 currentModTime);

    // Получить количество записей
    int count() const;

private:
    MetadataCache();
    ~MetadataCache();

    QMutex mutex_;
    bool initialized_ = false;

    void createTables();
    void createIndexes();
};
