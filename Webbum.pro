#-------------------------------------------------
#
# Project created by QtCreator 2015-09-04T10:02:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

win32: INCLUDEPATH += $$PWD/ffmpeg/x64/include
unix: INCLUDEPATH += usr/include

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32
{
    LIBS += -L$$PWD/ffmpeg/x64/lib\
            -lavutil\
            -lavformat\
            -lavcodec\
            #-lavdevice\
            #-lswscale\
            #-ldsound
}

unix
{
    LIBS += -L/usr/lib/ffmpeg\
        -lavutil\
        -lavformat\
        -lavcodec\
        #-lavdevice\
        #-lswscale\
        #-ldsound
}
