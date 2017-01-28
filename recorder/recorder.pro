#-------------------------------------------------
#
# Project created by QtCreator 2016-10-12T21:43:39
#
#-------------------------------------------------

QT       += core gui widgets

CONFIG   += c++14 console link_pkgconfig link_prl

TARGET = KuemmelRecorder-Lite
TEMPLATE = app

LIBS += -lmp3lame

CONFIG(debug) {
    LIBS += -L../libsoundio -L../libsoundio/debug -lsoundio
} else {
    LIBS += -L../libsoundio -L../libsoundio/release -lsoundio
}
DEFINES += SOUNDIO_STATIC_LIBRARY

SOURCES += main.cpp\
    mainwindow.cpp \
    recording/lameencoderstream.cpp \
    recording/configuratorpane.cpp \
    recording/coordinator.cpp \
    recording/errorwidget.cpp \
    recording/fancyprogressbar.cpp \
    recording/levelcalculator.cpp \
    recording/statusview.cpp

HEADERS  += mainwindow.h \
    recording/lameencoderstream.h \
    recording/configuratorpane.h \
    recording/coordinator.h \
    recording/errorwidget.h \
    recording/fancyprogressbar.h \
    recording/levelcalculator.h \
    recording/statusview.h

FORMS    += mainwindow.ui \
    recording/recordingstatusview.ui \
    recording/recordingconfiguratorpane.ui

INCLUDEPATH += ../libsoundio/soundio