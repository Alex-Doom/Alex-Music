#include "MainWindow.h"

#include <QApplication>    // Основной класс приложения
#include <QDir>           // Работа с директориями
#include <QFile>          // Работа с файлами
#include <QIcon>          // Иконки

// Точка входа в приложение
int main(int argc, char *argv[]) {
    // Создание объекта приложения Qt
    QApplication app(argc, argv);
    app.setStyle("Fusion");  // Установка стиля Fusion (кросс-платформенный)

    // Установка иконки для приложения
    QString appIconDir = QCoreApplication::applicationDirPath();  // Папка с exe
    QString iconPath = appIconDir + "/app_icon.ico";  // Путь к иконке

    // проверка, существует ли файл иконки
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));  // установка иконки приложения
        qDebug() << "Иконка приложения загружена:" << iconPath;
    } else {
        qDebug() << "Файл иконки не найден:" << iconPath;
        qDebug() << "Пожалуйста, поместите app_icon.ico в папку с исполняемым файлом";
    }

    MainWindow window;
    window.show();  // Показываем окно

    // Запуск главного цикла обработки событий
    return app.exec();
}
