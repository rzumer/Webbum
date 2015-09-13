#-------------------------------------------------
#
# Project created by QtCreator 2015-09-04T10:02:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

INCLUDEPATH += $$PWD/ffmpeg/win/x64/include

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LIBS += -L$$PWD/ffmpeg/win/x64/lib\
        -lavcodec\
        -lavformat\
        -lavdevice\
        -lswscale\
        -lavutil\
        -ldsound
