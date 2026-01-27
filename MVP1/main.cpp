#include "MainWindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include "resource_finder.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Загрузка иконки
    QString iconPath = ResourceFinder::findIcon();

    if (!iconPath.isEmpty()) {
        app.setWindowIcon(QIcon(iconPath));
        qDebug() << "✓ Иконка загружена из:" << iconPath;
    } else {
        qDebug() << "✗ Иконка не найдена. Создаю временную...";
        QPixmap tempIcon(32, 32);
        tempIcon.fill(QColor(0, 120, 212)); // Синий цвет
        app.setWindowIcon(QIcon(tempIcon));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
