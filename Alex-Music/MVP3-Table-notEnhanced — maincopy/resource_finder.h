#pragma once
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

class ResourceFinder {
public:
    static QString findIcon() {
        QStringList paths = {
            QCoreApplication::applicationDirPath() + "/app_icon.ico",
            QCoreApplication::applicationDirPath() + "/icons/app_icon.ico",
            ":/icons/app_icon.ico"
        };
        for (const auto& p : paths) {
            if (QFileInfo::exists(p)) return p;
        }
        return QString();
    }

    static QString findDefaultCover() {
        QStringList paths = {
            QCoreApplication::applicationDirPath() + "/default.jpg",
            QCoreApplication::applicationDirPath() + "/images/default.jpg"
        };
        for (const auto& p : paths) {
            if (QFileInfo::exists(p)) return p;
        }
        return QString();
    }
};
