# Self-Made QMake project file for libsoundio. Replaces the original CMake build.
# FIXME: CoreAudio and Jack have been left out because I can't test them right now

CONFIG   += c11 static create_prl link_pkgconfig warn_off

TARGET = soundio
TEMPLATE = lib

LIBSOUNDIO_VERSION_MAJOR = 1
LIBSOUNDIO_VERSION_MINOR = 1
LIBSOUNDIO_VERSION_PATCH = 0
LIBSOUNDIO_VERSION  = $${LIBSOUNDIO_VERSION_MAJOR}.$${LIBSOUNDIO_VERSION_MINOR}.$${LIBSOUNDIO_VERSION_PATCH}
VERSION = $${LIBSOUNDIO_VERSION_MAJOR}.$${LIBSOUNDIO_VERSION_MINOR}.$${LIBSOUNDIO_VERSION_PATCH}

DEFINES += SOUNDIO_VERSION_MAJOR=$$LIBSOUNDIO_VERSION_MAJOR
DEFINES += SOUNDIO_VERSION_MINOR=$$LIBSOUNDIO_VERSION_MINOR
DEFINES += SOUNDIO_VERSION_PATCH=$$LIBSOUNDIO_VERSION_PATCH
DEFINES += SOUNDIO_VERSION_STRING=\\\"$$LIBSOUNDIO_VERSION\\\"
DEFINES += SOUNDIO_STATIC_LIBRARY

win32 {
    SOUNDIO_HAVE_WASAPI = true
    DEFINES += SOUNDIO_HAVE_WASAPI
} else {
    SOUNDIO_HAVE_WASAPI = false
}

packagesExist(libpulse) {
    PKGCONFIG += libpulse
    SOUNDIO_HAVE_PULSEAUDIO = true
    DEFINES += SOUNDIO_HAVE_PULSEAUDIO
} else {
    SOUNDIO_HAVE_PULSEAUDIO = false
}

packagesExist(alsa) {
    PKGCONFIG += alsa
    SOUNDIO_HAVE_ALSA = true
    DEFINES += SOUNDIO_HAVE_ALSA
} else {
    SOUNDIO_HAVE_ALSA = false
}

SOURCES += \
    src/soundio.c \
    src/util.c \
    src/os.c \
    src/dummy.c \
    src/channel_layout.c \
    src/ring_buffer.c

HEADERS += \
    src/util.h \
    src/os.h \
    src/dummy.h \
    src/ring_buffer.h \
    src/config.h \
    src/soundio_private.h \
    src/soundio_internal.h

HEADERS += \
    soundio/soundio.h \
    soundio/endian.h


$$SOUNDIO_HAVE_PULSEAUDIO {
    SOURCES += \
        src/pulseaudio.c
    HEADERS += \
        src/pulseaudio.h
}

$$SOUNDIO_HAVE_ALSA {
    SOURCES += \
        src/alsa.c
    HEADERS += \
        src/alsa.h
}

$$SOUNDIO_HAVE_WASAPI {
    SOURCES += \
        src/wasapi.c
    HEADERS += \
        src/alsa.h
}

QMAKE_CFLAGS += -std=c11 -fvisibility=hidden -D_REENTRANT -D_POSIX_C_SOURCE=200809L
QMAKE_CFLAGS_DEBUG -= -O0
QMAKE_CFLAGS_DEBUG -= -O1
QMAKE_CFLAGS_DEBUG -= -O2
QMAKE_CFLAGS_DEBUG -= -O3
QMAKE_CFLAGS_DEBUG *= -Og
