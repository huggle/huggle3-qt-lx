#-------------------------------------------------
#
# Project created by QtCreator 2019-05-25T11:42:58
#
#-------------------------------------------------

QT       += core gui xml network multimedia webengine webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Huggle
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += HUGGLE_WEBEN

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += object_parallel_to_source

SOURCES += \
        ../../src/3rd/libirc/libirc/*.cpp \
        ../../src/3rd/libirc/libircclient/*.cpp
#        ../yaml-cpp-wasm/src/*.cpp \
#        ../../src/huggle_core/*.cpp \
#        ../../src/huggle_ui/*.cpp \
#        ../../src/huggle_ui/web_engine/*.cpp \
#        ../../src/huggle/main.cpp

HEADERS += \
        ../../src/3rd/libirc/libirc/*.h \
        ../../src/3rd/libirc/libircclient/*.h
#        ../../src/huggle_core/*.hpp \
#        ../../src/huggle_ui/web_engine/*.hpp \
#        ../../src/huggle_ui/*.hpp

#FORMS += \
#        ../../src/huggle_ui/web_engine/*.ui \
#        ../../src/huggle_ui/*.ui

INCLUDEPATH += ../yaml-cpp-wasm/include \
               ../../src/3rd \
               ../../src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
