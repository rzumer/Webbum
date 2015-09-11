#-------------------------------------------------
#
# Project created by QtCreator 2015-09-04T10:02:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Webbum
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        ffmpeg/ffmpeg.c

HEADERS  += mainwindow.h\
         ffmpeg.h

FORMS    += mainwindow.ui
