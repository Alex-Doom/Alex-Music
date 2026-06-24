#include "MetadataCache.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>

MetadataCache& MetadataCache::instance() {
    static MetadataCache inst;
    return inst;
}

MetadataResult MetadataCache::get(const QString& path) {
    QMutexLocker lock(&mutex_);
    auto* cached = cache_.object(path);
    if (cached) return *cached;
    lock.unlock();

    auto result = readFromFile(path);

    lock.relock();
    cache_.insert(path, new MetadataResult(result));
    return result;
}

MetadataResult MetadataCache::readFromFile(const QString& path) {
    MetadataResult res;
    QMediaPlayer player;
    QAudioOutput audio;
    player.setAudioOutput(&audio);

    QEventLoop loop;
    QTimer::singleShot(1500, &loop, &QEventLoop::quit);

    connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto m = player.metaData();
        res.title = m.value(QMediaMetaData::Title).toString();
        res.artist = m.value(QMediaMetaData::Author).toString();
        res.album = m.value(QMediaMetaData::AlbumTitle).toString();
        res.genre = m.value(QMediaMetaData::Genre).toString();
        res.duration = player.duration() / 1000;

        auto date = m.value(QMediaMetaData::Date);
        if (date.isValid()) res.year = date.toDate().toString("yyyy");

        auto cover = m.value(QMediaMetaData::CoverArtImage);
        if (cover.isValid()) res.cover = cover.value<QImage>();

        res.valid = true;
        loop.quit();
    });

    player.setSource(QUrl::fromLocalFile(path));
    loop.exec();
    return res;
}

void MetadataCache::prefetch(const QStringList& paths) {
    for (const auto& path : paths) {
        QThreadPool::globalInstance()->start([path]() {
            MetadataCache::instance().get(path);
        });
    }
}
