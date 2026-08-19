// FastTagReader.cpp
#include "FastTagReader.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/textidentificationframe.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/popularimeterframe.h>
#include <taglib/audioproperties.h>

#include <QFileInfo>
#include <QBuffer>
#include <QtConcurrent/QtConcurrent>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringDecoder>
#else
#include <QTextCodec>
#endif

// ---------- Реализация FastTagReader ----------

TrackMetadata FastTagReader::readMetadata(const QString& filePath) {
    TrackMetadata metadata;

    // Проверяем существование файла
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        qDebug() << "FastTagReader: файл не существует:" << filePath;
        return metadata;
    }

    // Открываем файл с помощью taglib
    TagLib::MPEG::File file(filePath.toLocal8Bit().data());

    if (!file.isOpen() || !file.isValid()) {
        qDebug() << "FastTagReader: не удалось открыть файл:" << filePath;
        return metadata;
    }

    // Получаем ID3v2 тег
    TagLib::ID3v2::Tag* tag = file.ID3v2Tag();
    if (tag) {
        // Читаем основные поля
        metadata.title = decodeTaglibString(tag->title());
        metadata.artist = decodeTaglibString(tag->artist());
        metadata.album = decodeTaglibString(tag->album());
        metadata.genre = decodeTaglibString(tag->genre());

        if (tag->year() > 0) {
            metadata.year = QString::number(tag->year());
        }

        if (tag->track() > 0) {
            metadata.trackNumber = tag->track();
        }

        // Извлекаем обложку
        metadata.coverImage = extractCoverImage(tag);
    }

    // Получаем длительность из аудио свойств
    TagLib::AudioProperties* properties = file.audioProperties();
    if (properties) {
        metadata.duration = properties->lengthInSeconds();
    }

    // Проверяем валидность
    metadata.isValid = !metadata.title.isEmpty() || !metadata.artist.isEmpty();

    qDebug() << "FastTagReader: прочитан трек"
             << metadata.artist << "-" << metadata.title
             << "длительность:" << metadata.duration << "сек";

    return metadata;
}

bool FastTagReader::saveRating(const QString& filePath, double rating) {
    if (rating < 0 || rating > 5) {
        qDebug() << "FastTagReader: неверный рейтинг:" << rating;
        return false;
    }

    TagLib::MPEG::File file(filePath.toLocal8Bit().data());

    if (!file.isOpen() || !file.isValid()) {
        qDebug() << "FastTagReader: не удалось открыть файл для записи:" << filePath;
        return false;
    }

    TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);

    if (!tag) {
        qDebug() << "FastTagReader: не удалось создать ID3v2 тег";
        return false;
    }

    // Удаляем старые фреймы POPM
    TagLib::ID3v2::FrameList oldFrames = tag->frameList("POPM");
    for (auto frame : oldFrames) {
        tag->removeFrame(frame);
        delete frame;
    }

    // Создаем новый фрейм POPM (Popularimeter)
    TagLib::ID3v2::PopularimeterFrame* popm =
        new TagLib::ID3v2::PopularimeterFrame();

    // Конвертируем рейтинг 0-5 в 0-255
    unsigned char ratingValue = static_cast<unsigned char>(rating * 51);
    popm->setRating(ratingValue);

    // Устанавливаем email (опционально)
    popm->setEmail("alexmusic@player");

    tag->addFrame(popm);

    bool saved = file.save();

    if (saved) {
        qDebug() << "FastTagReader: рейтинг" << rating << "сохранен для" << filePath;
    } else {
        qDebug() << "FastTagReader: не удалось сохранить рейтинг для" << filePath;
    }

    return saved;
}

bool FastTagReader::saveCover(const QString& filePath, const QImage& cover) {
    if (cover.isNull()) {
        return false;
    }

    TagLib::MPEG::File file(filePath.toLocal8Bit().data());

    if (!file.isOpen() || !file.isValid()) {
        return false;
    }

    TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);

    if (!tag) {
        return false;
    }

    // Удаляем старые обложки
    TagLib::ID3v2::FrameList oldPictures = tag->frameList("APIC");
    for (auto frame : oldPictures) {
        tag->removeFrame(frame);
        delete frame;
    }

    // Создаем новый фрейм для обложки
    TagLib::ID3v2::AttachedPictureFrame* picture =
        new TagLib::ID3v2::AttachedPictureFrame();

    // Конвертируем QImage в JPEG
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    cover.save(&buffer, "JPEG", 90);
    buffer.close();

    if (imageData.isEmpty()) {
        delete picture;
        return false;
    }

    // Устанавливаем данные
    TagLib::ByteVector data(imageData.data(), imageData.size());
    picture->setPicture(data);
    picture->setMimeType("image/jpeg");
    picture->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
    picture->setDescription("Cover");

    tag->addFrame(picture);

    return file.save();
}

bool FastTagReader::isValidMp3(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || fileInfo.size() == 0) {
        return false;
    }

    if (!filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
        return false;
    }

    TagLib::MPEG::File file(filePath.toLocal8Bit().data());
    return file.isOpen() && file.isValid();
}

int FastTagReader::getDuration(const QString& filePath) {
    TagLib::MPEG::File file(filePath.toLocal8Bit().data());

    if (!file.isOpen() || !file.isValid()) {
        return 0;
    }

    TagLib::AudioProperties* properties = file.audioProperties();
    if (properties) {
        return properties->lengthInSeconds();
    }

    return 0;
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

// В FastTagReader.cpp — decodeTaglibString
QString FastTagReader::decodeTaglibString(const TagLib::String& str) {
    if (str.isEmpty()) {
        return QString();
    }

    QString result = QString::fromUtf8(str.toCString(true));

    // === ВАША ЛОГИКА: >1 спецсимвол = mojibake, 1 = валидный акцент ===
    int specialChars = countLatin1Supplement(result);
    if (specialChars <= 1) {
        return result.trimmed(); // 0 или 1 спецсимвол — оставляем как есть
    }

    // >1 спецсимвол — пробуем CP1251
    QByteArray latin1Bytes = result.toLatin1();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder("Windows-1251");
    QString fixed = decoder.decode(latin1Bytes);
#else
    QTextCodec* codec = QTextCodec::codecForName("Windows-1251");
    QString fixed = codec->toUnicode(latin1Bytes);
#endif

    // Проверяем, что получили кириллицу
    for (const QChar& c : fixed) {
        if (c.unicode() >= 0x0400 && c.unicode() <= 0x04FF) {
            qDebug() << "  [decodeTaglibString] Исправлено:" << result << "→" << fixed;
            return fixed.trimmed();
        }
    }

    return result.trimmed();
}

QImage FastTagReader::extractCoverImage(TagLib::ID3v2::Tag* tag) {
    if (!tag) {
        return QImage();
    }

    TagLib::ID3v2::FrameList pictures = tag->frameList("APIC");
    if (pictures.isEmpty()) {
        return QImage();
    }

    // Берем первую обложку
    TagLib::ID3v2::AttachedPictureFrame* picture =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(pictures.front());

    if (!picture || picture->picture().size() == 0) {
        return QImage();
    }

    // Конвертируем данные в QImage
    const TagLib::ByteVector& data = picture->picture();
    QByteArray imageData(data.data(), data.size());

    QImage image;
    if (image.loadFromData(imageData)) {
        return image;
    }

    return QImage();
}

// ---------- Реализация FastTagReaderWrapper ----------

FastTagReaderWrapper::FastTagReaderWrapper(QObject* parent)
    : QObject(parent) {
}

FastTagReaderWrapper::~FastTagReaderWrapper() {
    isCancelled_ = true;
}

void FastTagReaderWrapper::cancel() {
    isCancelled_ = true;
}

void FastTagReaderWrapper::readMetadataAsync(const QString& filePath) {
    auto future = QtConcurrent::run([this, filePath]() {
        doReadMetadata(filePath);
    });
    Q_UNUSED(future)
}

void FastTagReaderWrapper::readBatchAsync(const QStringList& files) {
    auto future = QtConcurrent::run([this, files]() {
        doBatchRead(files);
    });
    Q_UNUSED(future)
}

void FastTagReaderWrapper::doReadMetadata(const QString& filePath) {
    if (isCancelled_) return;

    TrackMetadata metadata = FastTagReader::readMetadata(filePath);
    if (!isCancelled_) {
        emit metadataReady(filePath, metadata);
    }
}

void FastTagReaderWrapper::doBatchRead(const QStringList& files) {
    int total = files.size();
    isCancelled_ = false;

    for (int i = 0; i < total; ++i) {
        if (isCancelled_) break;

        TrackMetadata metadata = FastTagReader::readMetadata(files[i]);
        if (!isCancelled_) {
            emit metadataReady(files[i], metadata);
            emit batchProgress(i + 1, total);
        }
    }

    if (!isCancelled_) {
        emit batchFinished();
    }
}
