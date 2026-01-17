#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QDebug>

// Простая функция для поиска ресурсов
QString findResource(const QString& relativePath) {
    QString appDir = QCoreApplication::applicationDirPath();

    // Ищем рядом с .exe
    QString path = QDir::cleanPath(appDir + "/" + relativePath);
    if (QFile::exists(path)) {
        return path;
    }

    // Если не нашли - выводим информацию
    qDebug() << "Ресурс не найден:" << path;
    return QString();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Ищем иконку
    QString iconPath = findResource("icons/app_icon.png");

    if (iconPath.isEmpty()) {
        // Пробуем .ico
        iconPath = findResource("icons/app_icon.ico");
    }

    if (!iconPath.isEmpty()) {
        app.setWindowIcon(QIcon(iconPath));
        qDebug() << "✓ Иконка загружена:" << iconPath;
    } else {
        qDebug() << "✗ Иконка не найдена. Создаю временную...";
        // Временная синяя иконка
        QPixmap tempIcon(32, 32);
        tempIcon.fill(QColor(0, 120, 212));
        app.setWindowIcon(QIcon(tempIcon));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
