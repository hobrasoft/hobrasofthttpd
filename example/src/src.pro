CODECFORTR = UTF-8
TEMPLATE = app
TARGET = example
DESTDIR = ../bin
LIBS += -lqjson -ldl

CONFIG -= stl
CONFIG += qt warn_on thread

QT += network
QT -= gui

include(../../global.pri)
include(application/application.pri)
include(../../src/hobrasofthttpd.pri)
include(httpd/httpd.pri)

SOURCES += main.cpp
