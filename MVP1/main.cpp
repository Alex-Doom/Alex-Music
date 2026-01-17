#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Способ 1: Использование ресурсов Qt (:/)
    // Эта иконка будет встроена в исполняемый файл
    QIcon iconFromResources(":/icons/app_icon.ico");

    if (!iconFromResources.isNull()) {
        app.setWindowIcon(iconFromResources);
        qDebug() << "✓ Иконка загружена из ресурсов Qt";
    } else {
        qDebug() << "✗ Иконка не найдена в ресурсах Qt";

        // Способ 2: Резервный вариант - ищем файл рядом с .exe
        QString appDir = QCoreApplication::applicationDirPath();
        QStringList possiblePaths = {
            appDir + "/app_icon.ico",
            appDir + "/../icons/app_icon.ico",
            appDir + "/../../icons/app_icon.ico",
            appDir + "/../../../icons/app_icon.ico",
        };

        bool found = false;
        // for (const QString& path : possiblePaths) {
        //     if (QFile::exists(path)) {
        //         app.setWindowIcon(QIcon(path));
        //         qDebug() << "✓ Иконка загружена из файла:" << QDir::toNativeSeparators(path);
        //         found = true;
        //         break;
        //     }
        // }

        // Способ 3: Создаем иконку программно
        if (!found) {
            qDebug() << "✗ Файл иконки не найден. Создаю временную иконку";
            QPixmap tempIcon(64, 64);
            tempIcon.fill(QColor(0, 120, 212)); // Синий цвет
            app.setWindowIcon(QIcon(tempIcon));
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
