#include "ScannerThread.h"
#include <QFileInfo>
#include <QDebug>

ScannerThread::ScannerThread(QObject* parent) : QThread(parent) {}

void ScannerThread::run() {
    if (scanPath_.isEmpty()) {
        emit scanError("Путь для сканирования не указан");
        return;
    }

    QDir dir(scanPath_);
    if (!dir.exists()) {
        emit scanError("Папка не существует: " + scanPath_);
        return;
    }

    QStringList filePaths;
    QDirIterator it(scanPath_, {"*.mp3"}, QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext() && !stop_) {
        filePaths.append(it.next());
    }

    if (stop_) return;

    int totalFiles = filePaths.size();
    emit scanProgress(0, totalFiles, "Начинаем сканирование...");

    if (totalFiles == 0) {
        emit scanError("MP3 файлы не найдены");
        return;
    }

    for (int i = 0; i < totalFiles && !stop_; ++i) {
        const QString& filePath = filePaths[i];
        QFileInfo fileInfo(filePath);

        if (!fileInfo.exists() || fileInfo.size() == 0) {
            emit scanProgress(i + 1, totalFiles, fileInfo.fileName());
            continue;
        }

        Track track(filePath.toStdString());

        // Быстрая загрузка из имени файла для немедленного отображения
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);

        if (parts.size() >= 2) {
            track.setArtist(parts[0].trimmed().toStdString());
            QString title = parts[1].trimmed();
            while (!title.isEmpty() && title[0].isDigit()) title.remove(0, 1);
            if (!title.isEmpty() && (title[0] == '_' || title[0] == '.' || title[0] == '-' || title[0] == ' ')) {
                title.remove(0, 1);
            }
            track.setTitle(title.toStdString());
        } else {
            QString title = baseName.trimmed();
            while (!title.isEmpty() && title[0].isDigit()) title.remove(0, 1);
            track.setTitle(title.toStdString());
            track.setArtist("Unknown Artist");
        }

        emit trackFound(track);
        emit scanProgress(i + 1, totalFiles, fileInfo.fileName());

        if (i % 10 == 0) msleep(1);
    }

    if (!stop_) emit scanFinished(totalFiles);
}
