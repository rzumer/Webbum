#include "controllers/mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
           QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&qtTranslator);

    QTranslator appTranslator;
    appTranslator.load("Webbum_" + QLocale::system().name());
    application.installTranslator(&appTranslator);

    MainWindow window;
    window.show();

    return application.exec();
}
