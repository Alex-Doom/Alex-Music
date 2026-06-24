#ifndef MP3METADATA_H
#define MP3METADATA_H

#include <QString>
#include <QImage>

struct Mp3Metadata {
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    int trackNumber = 0;
    int duration = 0;
    QImage coverImage;

    static Mp3Metadata fromFile(const QString& filePath);
    static QImage extractCover(const QString& filePath);
};

#endif // MP3METADATA_H
