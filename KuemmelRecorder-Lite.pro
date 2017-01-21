#-------------------------------------------------
#
# Project created by QtCreator 2016-10-12T21:43:39
#
#-------------------------------------------------

QT       += core gui widgets

CONFIG   += c++14 console link_pkgconfig

TARGET = KuemmelRecorder-Lite
TEMPLATE = app

LIBS += -lmp3lame

PKGCONFIG += portaudio-2.0

SOURCES += main.cpp\
    mainwindow.cpp \
    recordingcoordinator.cpp \
    lameencoderstream.cpp \
    external/pa_ringbuffer.c \
    recordingstatusview.cpp \
    recordingconfiguratorpane.cpp \
    recordingerrorwidget.cpp \
    recordinglevelcalculator.cpp \
    recordingfancyprogressbar.cpp

HEADERS  += mainwindow.h \
    recordingcoordinator.h \
    lameencoderstream.h \
    external/pa_memorybarrier.h \
    external/pa_ringbuffer.h \
    recordingstatusview.h \
    recordingconfiguratorpane.h \
    recordingerrorwidget.h \
    recordinglevelcalculator.h \
    recordingfancyprogressbar.h

FORMS    += mainwindow.ui \
    recordingstatusview.ui \
    recordingconfiguratorpane.ui
