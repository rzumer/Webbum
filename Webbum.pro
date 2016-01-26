#-------------------------------------------------
#
# Project created by QtCreator 2015-09-04T10:02:12
#
#-------------------------------------------------

QT       += core gui winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

win32:RC_ICONS += $$PWD/icon64.ico

win32
{
    !contains(QMAKE_TARGET.arch, x86_64) {
        INCLUDEPATH += $$PWD/ffmpeg/x86/include
    } else {
        INCLUDEPATH += $$PWD/ffmpeg/x64/include
    }
}
unix: INCLUDEPATH += usr/include

SOURCES += main.cpp\
        mainwindow.cpp \
    inputfile.cpp \
    outputfile.cpp \
    inputstream.cpp \
    inputchapter.cpp

HEADERS  += mainwindow.h \
    outputfile.h \
    inputfile.h \
    inputstream.h \
    inputchapter.h

FORMS    += mainwindow.ui

win32
{
    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")

        LIBS += -L$$PWD/ffmpeg/x86/lib\
                -lavutil\
                -lavformat\
                -lavcodec\
                #-lavdevice\
                #-lswscale\
                #-ldsound
    } else {
        message("x86_64 build")

        LIBS += -L$$PWD/ffmpeg/x64/lib\
                -lavutil\
                -lavformat\
                -lavcodec\
                #-lavdevice\
                #-lswscale\
                #-ldsound
    }
}

unix {
    LIBS += -L/usr/lib/ffmpeg\
        -lavutil\
        -lavformat\
        -lavcodec\
        #-lavdevice\
        #-lswscale\
        #-ldsound
}
