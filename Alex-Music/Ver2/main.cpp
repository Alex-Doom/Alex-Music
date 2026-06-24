#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Проверяем наличие обложки по умолчанию
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultCover = appDir + "/default_cover.jpg";

    if (!QFile::exists(defaultCover)) {
        qDebug() << "Default cover not found at:" << defaultCover;
        qDebug() << "Please place default_cover.jpg in application directory";
    }

    QString appIconDir = QCoreApplication::applicationDirPath();
    QString iconPath = appIconDir + "/app_icon.ico";

    // Проверяем существует ли файл иконки
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
        qDebug() << "Иконка загружена:" << iconPath;
    } else {
        qDebug() << "Файл иконки не найден:" << iconPath;
        qDebug() << "Пожалуйста, поместите app_icon.ico в папку с исполняемым файлом";
    }

    MainWindow window;

    // Также устанавливаем иконку для главного окна
    if (QFile::exists(iconPath)) {
        window.setWindowIcon(QIcon(iconPath));
    }

    window.show();

    return app.exec();
}
