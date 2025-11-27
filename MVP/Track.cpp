#include "Track.h"
#include <QDir>           // Работа с директориями
#include <QFileInfo>      // Информация о файле
#include <QFile>          // Работа с файлами
#include <QImage>         // Работа с изображениями
#include <QMediaMetaData> // Метаданные медиа
#include <QMediaPlayer>   // Медиаплеер
#include <QBuffer>        // Буфер в памяти
#include <QEventLoop>     // Цикл событий для асинхронных операций
#include <QTimer>         // Таймер

Track::Track(std::string path, std::string artist, std::string title,
             std::string album, double rating)
    : path_(std::move(path)), artist_(std::move(artist)),
    title_(std::move(title)), album_(std::move(album)), rating_(rating) {}

// Установка обложки: либо из MP3 файла, либо установка по умолчанию
QImage Track::getCoverImage() const {
    // Пытаемся извлечь обложку из MP3 файла
    QImage cover = extractCoverFromMP3();

    // обложка по умолчанию
    if (cover.isNull()) {
        cover = loadDefaultCover();
    }

    return cover;
}

// МЕТОД 1: извлечение обложки из MP3
QImage Track::extractCoverFromMP3() const {
    QImage cover;

    QMediaPlayer player; // медиаплеер для извлечения метаданных
    player.setSource(QUrl::fromLocalFile(QString::fromStdString(path_)));

    QEventLoop loop; // цикл событий для ожидания загрузки метаданных
    QTimer timer; // таймер для ограничения времени ожидания
    timer.setSingleShot(true); // сработает 1 раз

    // Подключение к сигналу изменения метаданных
    QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
        QVariant coverData = player.metaData().value(QMediaMetaData::CoverArtImage);
        if (coverData.isValid()) {
            cover = coverData.value<QImage>();
            if (!cover.isNull()) {
                loop.quit();
            }
        }

        // Подключаем таймер: если время вышло - выходим из цикла
        if (cover.isNull()) {
            QVariant thumbnailData = player.metaData().value(QMediaMetaData::ThumbnailImage);
            if (thumbnailData.isValid()) {
                cover = thumbnailData.value<QImage>();
                if (!cover.isNull()) {
                    loop.quit();
                }
            }
        }
    });

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(500);

    // Запуск цикла событий (блокирующий вызов)
    loop.exec();

    return cover; // Возврат найденной обложки или пустого изображения
}

// МЕТОД 2: загрузка обложки по умолчанию
QImage Track::loadDefaultCover() const {
    // Строго заданный путь к обложке по умолчанию
    QString defaultCoverPath = "C:/My_QT/CPP/Alex_Music/work/untitled/build/Desktop_Qt_6_9_3_MinGW_64_bit-Debug/default.jpg";

    // Попытка загрузить default.jpg
    if (QFile::exists(defaultCoverPath)) {
        QImage defaultCover(defaultCoverPath);
        if (!defaultCover.isNull()) {
            return defaultCover;
        }
    }

    // Если default.jpg не найден или не загрузился - возврат пустого изображения
    return QImage();
}

// Метод для получения уникального идентификатора трека (путь к файлу)
std::string Track::getID() const {
    return path_;
}
