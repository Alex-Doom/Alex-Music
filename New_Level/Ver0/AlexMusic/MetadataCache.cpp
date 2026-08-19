// #include "MetadataCache.h"
// // #include <QSqlQuery>
// // #include <QSqlError>
// #include <QFileInfo>
// #include <QDir>
// #include <QDebug>
// #include <QStandardPaths>

// MetadataCache::MetadataCache() {}
// MetadataCache::~MetadataCache() { close(); }

// MetadataCache& MetadataCache::instance() {
//     static MetadataCache cache;
//     return cache;
// }

// bool MetadataCache::initialize(const QString& dbPath) {
//     QMutexLocker lock(&mutex_);
//     if (initialized_) return true;

//     // db_ = QSqlDatabase::addDatabase("QSQLITE");
//     // db_.setDatabaseName(dbPath);

//     // if (!db_.open()) {
//     //     qWarning() << "Не удалось открыть БД кэша:" << db_.lastError().text();
//     //     return false;
//     // }

//     createTables();
//     createIndexes();
//     initialized_ = true;
//     qDebug() << "✅ MetadataCache инициализирован:" << dbPath;
//     return true;
// }

// void MetadataCache::close() {
//     QMutexLocker lock(&mutex_);
//     if (db_.isOpen()) {
//         db_.close();
//     }
//     initialized_ = false;
// }

// void MetadataCache::createTables() {
//     QSqlQuery query(db_);
//     query.exec(R"(
//         CREATE TABLE IF NOT EXISTS tracks (
//             file_path       TEXT PRIMARY KEY,
//             folder_path     TEXT NOT NULL,
//             title           TEXT,
//             artist          TEXT,
//             album           TEXT,
//             genre           TEXT,
//             year            TEXT,
//             duration        INTEGER DEFAULT 0,
//             file_size       INTEGER DEFAULT 0,
//             rating          REAL DEFAULT 0.0,
//             file_mod_time   INTEGER DEFAULT 0,
//             is_valid        INTEGER DEFAULT 1,
//             cached_at       INTEGER DEFAULT 0
//         )
//     )");

//     if (query.lastError().isValid()) {
//         qWarning() << "Ошибка создания таблицы:" << query.lastError().text();
//     }
// }

// void MetadataCache::createIndexes() {
//     QSqlQuery query(db_);
//     query.exec("CREATE INDEX IF NOT EXISTS idx_folder ON tracks(folder_path)");
//     query.exec("CREATE INDEX IF NOT EXISTS idx_artist ON tracks(artist)");
//     query.exec("CREATE INDEX IF NOT EXISTS idx_title ON tracks(title)");
//     query.exec("CREATE INDEX IF NOT EXISTS idx_rating ON tracks(rating)");
// }

// QVector<CachedTrack> MetadataCache::loadFolder(const QString& folderPath) {
//     QMutexLocker lock(&mutex_);
//     QVector<CachedTrack> results;
//     if (!initialized_) return results;

//     QSqlQuery query(db_);
//     query.prepare("SELECT file_path, title, artist, album, genre, year, "
//                   "duration, file_size, rating, file_mod_time, is_valid "
//                   "FROM tracks WHERE folder_path = ?");
//     query.addBindValue(folderPath);

//     if (!query.exec()) {
//         qWarning() << "Ошибка запроса:" << query.lastError().text();
//         return results;
//     }

//     results.reserve(query.size());
//     while (query.next()) {
//         CachedTrack t;
//         t.filePath    = query.value(0).toString();
//         t.title       = query.value(1).toString();
//         t.artist      = query.value(2).toString();
//         t.album       = query.value(3).toString();
//         t.genre       = query.value(4).toString();
//         t.year        = query.value(5).toString();
//         t.duration    = query.value(6).toInt();
//         t.fileSize    = query.value(7).toLongLong();
//         t.rating      = query.value(8).toDouble();
//         t.fileModTime = query.value(9).toLongLong();
//         t.isValid     = query.value(10).toInt() == 1;
//         results.append(t);
//     }

//     qDebug() << "📦 Загружено из кэша:" << results.size() << "треков для" << folderPath;
//     return results;
// }

// bool MetadataCache::loadTrack(const QString& filePath, CachedTrack& track) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return false;

//     QSqlQuery query(db_);
//     query.prepare("SELECT title, artist, album, genre, year, duration, "
//                   "file_size, rating, file_mod_time, is_valid "
//                   "FROM tracks WHERE file_path = ?");
//     query.addBindValue(filePath);

//     if (!query.exec() || !query.next()) return false;

//     track.filePath    = filePath;
//     track.title       = query.value(0).toString();
//     track.artist      = query.value(1).toString();
//     track.album       = query.value(2).toString();
//     track.genre       = query.value(3).toString();
//     track.year        = query.value(4).toString();
//     track.duration    = query.value(5).toInt();
//     track.fileSize    = query.value(6).toLongLong();
//     track.rating      = query.value(7).toDouble();
//     track.fileModTime = query.value(8).toLongLong();
//     track.isValid     = query.value(9).toInt() == 1;
//     return true;
// }

// void MetadataCache::saveTrack(const CachedTrack& track) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return;

//     QSqlQuery query(db_);
//     query.prepare(R"(
//         INSERT OR REPLACE INTO tracks
//         (file_path, folder_path, title, artist, album, genre, year,
//          duration, file_size, rating, file_mod_time, is_valid, cached_at)
//         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
//     )");

//     QFileInfo fi(track.filePath);
//     query.addBindValue(track.filePath);
//     query.addBindValue(fi.absolutePath());
//     query.addBindValue(track.title);
//     query.addBindValue(track.artist);
//     query.addBindValue(track.album);
//     query.addBindValue(track.genre);
//     query.addBindValue(track.year);
//     query.addBindValue(track.duration);
//     query.addBindValue(track.fileSize);
//     query.addBindValue(track.rating);
//     query.addBindValue(track.fileModTime);
//     query.addBindValue(track.isValid ? 1 : 0);
//     query.addBindValue(QDateTime::currentMSecsSinceEpoch());

//     if (!query.exec()) {
//         qWarning() << "Ошибка сохранения трека:" << query.lastError().text();
//     }
// }

// void MetadataCache::saveTracks(const QVector<CachedTrack>& tracks) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_ || tracks.isEmpty()) return;

//     // Используем транзакцию для скорости (в 50-100 раз быстрее)
//     db_.transaction();

//     QSqlQuery query(db_);
//     query.prepare(R"(
//         INSERT OR REPLACE INTO tracks
//         (file_path, folder_path, title, artist, album, genre, year,
//          duration, file_size, rating, file_mod_time, is_valid, cached_at)
//         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
//     )");

//     for (const auto& track : tracks) {
//         QFileInfo fi(track.filePath);
//         query.addBindValue(track.filePath);
//         query.addBindValue(fi.absolutePath());
//         query.addBindValue(track.title);
//         query.addBindValue(track.artist);
//         query.addBindValue(track.album);
//         query.addBindValue(track.genre);
//         query.addBindValue(track.year);
//         query.addBindValue(track.duration);
//         query.addBindValue(track.fileSize);
//         query.addBindValue(track.rating);
//         query.addBindValue(track.fileModTime);
//         query.addBindValue(track.isValid ? 1 : 0);
//         query.addBindValue(QDateTime::currentMSecsSinceEpoch());
//         query.exec();
//     }

//     db_.commit();
//     qDebug() << "💾 Сохранено в кэш:" << tracks.size() << "треков";
// }

// void MetadataCache::removeTrack(const QString& filePath) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return;
//     QSqlQuery query(db_);
//     query.prepare("DELETE FROM tracks WHERE file_path = ?");
//     query.addBindValue(filePath);
//     query.exec();
// }

// void MetadataCache::clearFolder(const QString& folderPath) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return;
//     QSqlQuery query(db_);
//     query.prepare("DELETE FROM tracks WHERE folder_path = ?");
//     query.addBindValue(folderPath);
//     query.exec();
// }

// bool MetadataCache::isCacheValid(const QString& filePath, qint64 currentModTime) {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return false;

//     QSqlQuery query(db_);
//     query.prepare("SELECT file_mod_time FROM tracks WHERE file_path = ?");
//     query.addBindValue(filePath);

//     if (!query.exec() || !query.next()) return false;
//     qint64 cachedModTime = query.value(0).toLongLong();
//     return cachedModTime == currentModTime;
// }

// int MetadataCache::count() const {
//     QMutexLocker lock(&mutex_);
//     if (!initialized_) return 0;
//     QSqlQuery query(db_);
//     query.exec("SELECT COUNT(*) FROM tracks");
//     if (query.next()) return query.value(0).toInt();
//     return 0;
// }
