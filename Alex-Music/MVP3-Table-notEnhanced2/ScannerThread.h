#pragma once
#include <QThread>
#include <QDirIterator>
#include <QStringList>
#include <atomic>
#include "Track.h"

class ScannerThread : public QThread {
    Q_OBJECT
public:
    explicit ScannerThread(QObject* parent = nullptr);
    void setScanPath(const QString& path) { scanPath_ = path; }
    void stop() { stop_ = true; }

signals:
    void trackFound(const Track& track);
    void scanProgress(int current, int total, const QString& currentFile);
    void scanFinished(int totalTracks);
    void scanError(const QString& error);

protected:
    void run() override;

private:
    QString scanPath_;
    std::atomic<bool> stop_{false};
};
