CODECFORTR = UTF-8
TEMPLATE = app
TARGET = test
DESTDIR = ../bin
INCLUDEPATH += ..

CONFIG -= stl
CONFIG += qt warn_on thread help qtestlib

equals(QT_ARCH, arm) {
  DEFINES += ARM
}

QT += network testlib
QT -= sql gui

include(../global.pri)
include(../src/hobrasofthttpd.pri)

RESOURCES += \
 test.qrc

HEADERS += \
 test.h \

SOURCES += \
 main.cpp \
 test.cpp \

