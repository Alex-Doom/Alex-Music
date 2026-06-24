#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>

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

    MainWindow window;
    window.show();

    return app.exec();
}
