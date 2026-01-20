#-------------------------------------------------
#
# Project created by QtCreator 2016-10-12T21:43:39
#
#-------------------------------------------------

QT       += core gui widgets svg

CONFIG   += c++14 link_pkgconfig

TARGET = KuemmelRecorder-Lite
TEMPLATE = app

PKGCONFIG += openal flac lame

QMAKE_CXXFLAGS_DEBUG -= -O0
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3
QMAKE_CXXFLAGS_DEBUG *= -Og

SOURCES += main.cpp\
    logger.cpp \
    mainwindow.cpp \
    recording/lameencoderstream.cpp \
    recording/configuratorpane.cpp \
    recording/coordinator.cpp \
    recording/errorwidget.cpp \
    recording/fancyprogressbar.cpp \
    recording/levelcalculator.cpp \
    recording/statusview.cpp \
    recording/lastfilepane.cpp \
    recording/backend.cpp \
    recording/abstractencoderstream.cpp \
    recording/flacencoderstream.cpp \
    recording/utilopenfile.cpp

HEADERS  += mainwindow.h \
    logger.h \
    recording/lameencoderstream.h \
    recording/configuratorpane.h \
    recording/coordinator.h \
    recording/errorwidget.h \
    recording/fancyprogressbar.h \
    recording/levelcalculator.h \
    recording/statusview.h \
    recording/lastfilepane.h \
    recording/backend.h \
    recording/abstractencoderstream.h \
    recording/flacencoderstream.h \
    recording/utilopenfile.h

FORMS    += mainwindow.ui \
    recording/recordingstatusview.ui \
    recording/recordingconfiguratorpane.ui \
    recording/lastfilepane.ui

RESOURCES += \
    recorder.qrc

TRANSLATIONS += l10n/recorder_de.ts

win32 {
    RC_ICONS = img/fishy.ico
}

packagesExist(mxe-licenses-html) {
    DEFINES += HAVE_THIRDPARTY_LICENSES
    PKGCONFIG += mxe-licenses-html zlib
    SOURCES += thirdpartylicensedialog.cpp
    HEADERS += thirdpartylicensedialog.h
    FORMS +=  thirdpartylicensedialog.ui
}
