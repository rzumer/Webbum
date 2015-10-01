#include "mainwindow.h"
#include <QApplication>
/*#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>*/

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /*QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
           QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(qtTranslator);

    QTranslator appTranslator;
    appTranslator.load("Webbum_" + QLocale::system().name());
    a.installTranslator(appTranslator);*/

    MainWindow w;
    w.show();

    return a.exec();
}
