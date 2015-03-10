TEMPLATE = lib
CONFIG += singleapplication-buildlib
QT += core
QT -= gui

include(libSingleApplication.pri)

TARGET = $$LIBSINGLEAPPLICATION_NAME
DESTDIR = $$PROJECT_LIBDIR
