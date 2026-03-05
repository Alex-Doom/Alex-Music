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

    // Сначала собираем все файлы
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

    // Обрабатываем файлы
    for (int i = 0; i < totalFiles && !stop_; ++i) {
        const QString& filePath = filePaths[i];
        QFileInfo fileInfo(filePath);

        // Быстрая проверка
        if (!fileInfo.exists() || fileInfo.size() == 0) {
            emit scanProgress(i + 1, totalFiles, fileInfo.fileName());
            continue;
        }

        // Создаем трек с БАЗОВЫМИ данными (без полной загрузки метаданных)
        Track track(filePath.toStdString());

        // ТОЛЬКО имя файла для быстрого отображения
        QString baseName = fileInfo.baseName();
        QStringList parts = baseName.split(" - ", Qt::SkipEmptyParts);
        QString artist = parts.value(0, "Unknown Artist");
        QString title = parts.value(1, baseName);

        track.setArtist(artist.toStdString());
        track.setTitle(title.toStdString());

        // Отправляем трек для немедленного отображения
        emit trackFound(track);

        emit scanProgress(i + 1, totalFiles, fileInfo.fileName());

        // Небольшая пауза для отзывчивости UI
        if (i % 10 == 0) {
            msleep(1);
        }
    }

    if (!stop_) {
        emit scanFinished(totalFiles);
    }
}
