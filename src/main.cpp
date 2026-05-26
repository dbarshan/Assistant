#include <QApplication>
#include <QIcon>
#include <QDir>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // Set standard Chromium parameters for high performance
    // and smooth hardware acceleration.
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    QApplication app(argc, argv);

    // Set Application Identity
    QApplication::setApplicationName("Assistant");
    QApplication::setOrganizationName("Antigravity");
    QApplication::setApplicationVersion("1.0.0");

    // Set Application Icon globally for taskbar/task panel integration
    app.setWindowIcon(QIcon(QDir::currentPath() + "/resources/icon.png"));

    MainWindow window;
    window.show();

    return app.exec();
}
