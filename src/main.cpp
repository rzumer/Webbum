#include "controllers/mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setApplicationName("Webbum");

    QTranslator translator;
    QString locale = QLocale::system().name();
    translator.load(application.applicationName() + "_" + locale);
    application.installTranslator(&translator);

    MainWindow window;
    window.show();

    return application.exec();
}
