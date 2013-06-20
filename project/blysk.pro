#-------------------------------------------------
#
# Project created by QtCreator 2013-06-20T15:56:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = blysk
TEMPLATE = app

SOURCES += \
    ../src/Audio/AudioWave.cpp \
    ../src/Audio/AudioDevice.cpp \
    ../src/main.cpp \
    ../src/System/RtMidi.cpp \
    ../src/System/RtAudio.cpp \
    ../src/UI/MainWindow.cpp \
    ../src/Maths/chuck_fft.c \
    ../src/Engine/Item.cpp

HEADERS += \
    ../src/Audio/HasChannelsAndSampleRate.h \
    ../src/Audio/AudioWave.h \
    ../src/Audio/AudioRenderInterface.h \
    ../src/Audio/AudioInputInterface.h \
    ../src/Audio/AudioException.h \
    ../src/Audio/AudioDevice.h \
    ../src/System/RtMidi.h \
    ../src/System/RtError.h \
    ../src/System/RtAudio.h \
    ../src/UI/MainWindow.h \
    ../src/Maths/chuck_fft.h \
    ../src/Engine/Item.h

FORMS += \
    ../src/UI/MainWindow.ui

macx: QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
macx: QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden -fvisibility-inlines-hidden

INCLUDEPATH += ../../boost_1_53_0/
DEPENDPATH += ../../boost_1_53_0/

INCLUDEPATH += ../src
DEPENDPATH += ../src

win32: LIBS += -L"C:/Users/jarek/Coding/boost_1_53_0/stage/lib/" libboost_system-mgw47-mt-1_53 libboost_filesystem-mgw47-mt-1_53
win32: LIBS += -L"C:/Program Files (x86)/Mega-Nerd/libsndfile/lib/" -llibsndfile-1

win32: LIBS += -lole32 -luser32
CONFIG+=c++11

macx: LIBPATH += ../../libsndfile-1.0.24/src ../../libsamplerate-0.1.8/src ../../boost_1_53_0/stage/lib

unix: LIBS += -lboost_system -lboost_filesystem -lsndfile -lsamplerate
QMAKE_CXXFLAGS += -std=c++11

macx: LIBS += -framework CoreFoundation -framework CoreAudio -framework CoreMidi

macx: INCLUDEPATH += ../../libsndfile-1.0.24/src ../../libsamplerate-0.1.8/src

win32: INCLUDEPATH += "C:/Program Files (x86)/Mega-Nerd/libsndfile/include"
win32: DEPENDPATH += "C:/Program Files (x86)/Mega-Nerd/libsndfile/include"

win32: DEFINES += __WINDOWS_DS__ NOOMP
unix:!macx: DEFINES += __LINUX_ALSA__
macx: DEFINES += __MACOSX_CORE__

OTHER_FILES += \
    ../README.txt


