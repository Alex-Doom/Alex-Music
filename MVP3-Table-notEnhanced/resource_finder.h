// resource_finder.h
#pragma once
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

class ResourceFinder {
public:
    static QString findResource(const QString& relativePath) {
        QString appDir = QCoreApplication::applicationDirPath();

        // Список возможных расположений ресурсов
        QStringList possiblePaths = {
            QDir::cleanPath(appDir + "/" + relativePath),                    // Рядом с .exe
            QDir::cleanPath(appDir + "/../" + relativePath),                 // На уровень выше
            QDir::cleanPath(appDir + "/../../" + relativePath),              // На 2 уровня выше
            QDir::cleanPath(appDir + "/../../Alex-Music/MVP1/" + relativePath), // В папке проекта
            QDir::cleanPath(appDir + "/../../" + relativePath),              // В корне проекта
        };

        for (const QString& path : possiblePaths) {
            if (QFileInfo::exists(path)) {
                qDebug() << "Найден ресурс:" << path;
                return path;
            }
        }

        qDebug() << "Ресурс не найден:" << relativePath;
        qDebug() << "Искали по путям:";
        for (const QString& path : possiblePaths) {
            qDebug() << "  " << path;
        }

        return QString();
    }

    static QString findIcon() {
        QStringList possibleNames = {
            "app_icon.ico",
            "icons/app_icon.ico",
            "icons/app_icon.png",
            "../icons/app_icon.ico",
            "../../icons/app_icon.ico"
        };

        for (const QString& name : possibleNames) {
            QString path = findResource(name);
            if (!path.isEmpty()) return path;
        }

        return QString();
    }

    static QString findDefaultCover() {
        QStringList possibleNames = {
            "default.jpg",
            "images/default.jpg",
            "../images/default.jpg",
            "../../images/default.jpg",
            "icons/default.jpg"
        };

        for (const QString& name : possibleNames) {
            QString path = findResource(name);
            if (!path.isEmpty()) return path;
        }

        return QString();
    }
};
