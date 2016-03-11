CODECFORTR = UTF-8
TEMPLATE = app
TARGET = example
DESTDIR = ../bin

equals(QT_MAJOR_VERSION, 4) {
    LIBS += -lqjson
    }

CONFIG -= stl
CONFIG += qt warn_on thread

QT += network
QT -= gui

include(../../global.pri)
include(application/application.pri)
include(../../src/hobrasofthttpd.pri)
include(httpd/httpd.pri)

SOURCES += main.cpp
