#-------------------------------------------------
#
# Project created by QtCreator 2016-10-12T21:43:39
#
#-------------------------------------------------

QT       += core gui widgets svg

CONFIG   += c++14 console link_pkgconfig

TARGET = KuemmelRecorder-Lite
TEMPLATE = app

LIBS += -lmp3lame

PKGCONFIG += openal

QMAKE_CXXFLAGS_DEBUG -= -O0
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3
QMAKE_CXXFLAGS_DEBUG *= -Og

DEFINES += GIT_REVNO="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" describe --tags --always)\\\""

SOURCES += main.cpp\
    mainwindow.cpp \
    recording/lameencoderstream.cpp \
    recording/configuratorpane.cpp \
    recording/coordinator.cpp \
    recording/errorwidget.cpp \
    recording/fancyprogressbar.cpp \
    recording/levelcalculator.cpp \
    recording/statusview.cpp \
    recording/lastfilepane.cpp \
    recording/backend.cpp

HEADERS  += mainwindow.h \
    recording/lameencoderstream.h \
    recording/configuratorpane.h \
    recording/coordinator.h \
    recording/errorwidget.h \
    recording/fancyprogressbar.h \
    recording/levelcalculator.h \
    recording/statusview.h \
    recording/lastfilepane.h \
    recording/backend.h

FORMS    += mainwindow.ui \
    recording/recordingstatusview.ui \
    recording/recordingconfiguratorpane.ui \
    recording/lastfilepane.ui

INCLUDEPATH += ../libsoundio/soundio

RESOURCES += \
    recorder.qrc

TRANSLATIONS += l10n/recorder_de.ts

win32 {
    RC_ICONS = img/fishy.ico
}
