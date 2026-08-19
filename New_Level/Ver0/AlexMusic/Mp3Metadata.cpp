// Mp3Metadata.cpp
#include "Mp3Metadata.h"
#include <QFileInfo>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QMediaMetaData>
#include <QRegularExpression>
#include <QString>

Mp3Metadata Mp3Metadata::fromFile(const QString& filePath) {
    Mp3Metadata metadata;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        qDebug() << "Файл не существует или пуст:" << filePath;
        metadata.title = fileInfo.baseName();
        metadata.artist = "Unknown Artist";
        metadata.album = "Unknown Album";
        return metadata;
    }

    QMediaPlayer player;
    QAudioOutput audioOutput;
    player.setAudioOutput(&audioOutput);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);

    bool metadataReceived = false;

    // Подключаем сигнал metaDataChanged
    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        auto metaData = player.metaData();

        // Заголовок - используем правильные ключи Qt6
        //metadata.title = metaData.value(QMediaMetaData::Title).toString();
        // |
        // >
        QVariant titleVariant = metaData.value (QMediaMetaData::Title);
        if (titleVariant.isValid()) {
            QByteArray titleData = titleVariant.toByteArray();
            metadata.title = decodeUtf16String(titleData);
        }

        if (metadata.title.isEmpty()) {
            // Альтернативные ключи
            metadata.title = metaData.value(QMediaMetaData::AlbumTitle).toString();
        }

        // Исполнитель
        metadata.artist = metaData.value(QMediaMetaData::Author).toString();
        if (metadata.artist.isEmpty()) {
            metadata.artist = metaData.value(QMediaMetaData::AlbumArtist).toString();
        }
        if (metadata.artist.isEmpty()) {
            metadata.artist = metaData.value(QMediaMetaData::ContributingArtist).toString();
        }

        // Альбом
        metadata.album = metaData.value(QMediaMetaData::AlbumTitle).toString();

        // Жанр
        metadata.genre = metaData.value(QMediaMetaData::Genre).toString();

        // Год - используем Date, так как Year нет в Qt6
        QVariant yearVariant = metaData.value(QMediaMetaData::Date);
        if (yearVariant.isValid()) {
            metadata.year = yearVariant.toString();
            // Если год в формате полной даты, берем только первые 4 символа
            if (metadata.year.length() > 4) {
                metadata.year = metadata.year.left(4);
            }
        }

        // Номер трека
        QVariant trackVariant = metaData.value(QMediaMetaData::TrackNumber);
        if (trackVariant.isValid()) {
            metadata.trackNumber = trackVariant.toInt();
        }

        // Обложка
        QVariant coverData = metaData.value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) {
            metadata.coverImage = coverData.value<QImage>();
        }

        if (metadata.coverImage.isNull()) {
            QVariant thumbnailData = metaData.value(QMediaMetaData::ThumbnailImage);
            if (thumbnailData.isValid()) {
                metadata.coverImage = thumbnailData.value<QImage>();
            }
        }

        metadataReceived = true;
        loop.quit();
    });

    // Подключаем сигнал durationChanged для получения длительности
    QObject::connect(&player, &QMediaPlayer::durationChanged, [&](qint64 duration) {
        metadata.duration = duration / 1000;
    });

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        if (!metadataReceived) {
            qDebug() << "Таймаут загрузки метаданных для:" << filePath;
            loop.quit();
        }
    });

    // Устанавливаем источник
    player.setSource(QUrl::fromLocalFile(filePath));

    // Ждем загрузки
    loop.exec();

    // Если метаданные не получены, используем имя файла
    if (!metadataReceived || metadata.title.isEmpty()) {
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

        if (parts.size() >= 2) {
            metadata.artist = parts[0].trimmed();
            metadata.title = parts[1].trimmed();
        } else {
            metadata.title = baseName.trimmed();
            metadata.artist = "Unknown Artist";
        }

        metadata.album = "Unknown Album";
        metadata.genre = "Unknown";

        qDebug() << "Использованы данные из имени файла:" << metadata.artist << "-" << metadata.title;
    }

    // Очищаем от лишних символов - исправляем создание QRegularExpression
    // Удаляем цифры, подчеркивания, точки и дефисы из начала строки
    while (!metadata.title.isEmpty() &&
           (metadata.title[0].isDigit() ||
            metadata.title[0] == '_' ||
            metadata.title[0] == '.' ||
            metadata.title[0] == '-' ||
            metadata.title[0] == ' ')) {
        metadata.title.remove(0, 1);
    }

    if (metadata.artist.isEmpty()) metadata.artist = "Unknown Artist";
    if (metadata.title.isEmpty()) metadata.title = "Unknown Title";

    return metadata;
}

QImage Mp3Metadata::extractCover(const QString& filePath) {
    Mp3Metadata metadata = fromFile(filePath);
    return metadata.coverImage;
}

static QString decodeUtf16String(const QByteArray& data) {
    if (data.size() < 2) return QString();

    // BOM check
    if (data.startsWith("\xFF\xFE")) {
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData() + 2), (data.size() - 2) / 2);
    } else if (data.startsWith("\xFE\xFF")) {
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData() + 2), (data.size() - 2) / 2);
    }

    // === ИСПРАВЛЕНИЕ: пробуем Windows-1251 перед UTF-16 ===
    // Многие русские MP3 используют ID3v2.3 с Windows-1251 без BOM
    QString result = QString::fromUtf8(data);
    if (result.contains("Ð") || result.contains("Ñ") || result.contains("Ã")) {
        // Похоже на Latin-1 интерпретацию UTF-8 — пробуем Windows-1251
        result = QString::fromLocal8Bit(data);
    }

    // Если всё ещё кракозябры — пробуем UTF-16 LE без BOM
    if (result.isEmpty() || (result[0].unicode() < 0x20 && data.size() > 2)) {
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), data.size() / 2);
    }

    return result;
}
