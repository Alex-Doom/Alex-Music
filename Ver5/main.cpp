#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Устанавливаем иконку для приложения
    QString appIconDir = QCoreApplication::applicationDirPath();
    QString iconPath = appIconDir + "/app_icon.ico";

    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
        qDebug() << "Иконка приложения загружена:" << iconPath;
    } else {
        qDebug() << "Файл иконки не найден:" << iconPath;
        qDebug() << "Пожалуйста, поместите app_icon.ico в папку с исполняемым файлом";
    }

    MainWindow window;
    // Иконка для окна уже устанавливается в конструкторе MainWindow
    window.show();

    return app.exec();
}
