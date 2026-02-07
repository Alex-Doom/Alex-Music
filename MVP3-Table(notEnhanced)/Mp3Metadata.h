#ifndef MP3METADATA_H
#define MP3METADATA_H

#include <QString>
#include <QImage>
#include <map>
#include <string>

struct Mp3Metadata {
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    int trackNumber = 0;
    int duration = 0; // в секундах
    QImage coverImage;

    // Статический метод для получения метаданных из файла
    static Mp3Metadata fromFile(const QString& filePath);

    // Статический метод для извлечения только обложки
    static QImage extractCover(const QString& filePath);
};

#endif // MP3METADATA_H
