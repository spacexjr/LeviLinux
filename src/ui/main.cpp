#include <QApplication>
#include "windows/MainWindow.hpp"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LeviLinux");
    app.setApplicationVersion("1.0.0");
    levi::MainWindow w;
    w.show();
    return app.exec();
}
