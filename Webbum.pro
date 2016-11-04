#-------------------------------------------------
#
# Project created by QtCreator 2015-09-04T10:02:12
#
#-------------------------------------------------

QT       += core gui winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

INCLUDEPATH += $$PWD/src

win32:RC_ICONS += $$PWD/icon64.ico

win32
{
    !contains(QMAKE_TARGET.arch, x86_64) {
        INCLUDEPATH += $$PWD/dependencies/ffmpeg/x86/include
    } else {
        INCLUDEPATH += $$PWD/dependencies/ffmpeg/x64/include
    }
}
unix: INCLUDEPATH += usr/include

SOURCES += src/main.cpp\
    src/controllers/mainwindow.cpp \
    src/controllers/ffmpegcontroller.cpp \
    src/models/inputfile.cpp \
    src/models/outputfile.cpp \
    src/models/inputstream.cpp \
    src/models/inputchapter.cpp

HEADERS  += src/controllers/mainwindow.h \
    src/controllers/ffmpegcontroller.h \
    src/models/outputfile.h \
    src/models/inputfile.h \
    src/models/inputstream.h \
    src/models/inputchapter.h

FORMS    += src/views/mainwindow.ui

TRANSLATIONS += Webbum_fr.ts

win32
{
    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")

        LIBS += -L$$PWD/dependencies/ffmpeg/x86/lib\
                -lavutil\
                -lavformat\
                -lavcodec\
                #-lavdevice\
                #-lswscale\
                #-ldsound
    } else {
        message("x86_64 build")

        LIBS += -L$$PWD/dependencies/ffmpeg/x64/lib\
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
