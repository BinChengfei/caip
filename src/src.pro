#-------------------------------------------------
#
# Project created by QtCreator 2014-04-16T22:49:56
#
#-------------------------------------------------

QT       += core gui network sql
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = caip
TEMPLATE = app

win32{
    COPY = copy \y
    MKDIR = mkdir
}else{
    COPY = cp
    MKDIR = mkdir -p
}

include(../3rd/SingleApplication/libSingleApplication.pri)
include(../3rd/Log4Qt/libLog4Qt.pri)

#FILES_COPY_SRC = log4qt.conf log4qt_devel.conf
#for(f, FILES_COPY_SRC){
#        src_file = $$PWD/$$f
#        dist = $$OUT_PWD
#        src_file = $$replace(src_file, /, \\)
#        dist = $$replace(dist, /, \\)

#        !exists($$dist):system($$MKDIR $$dist)
#        system($$COPY $$src_file $$dist)
#}

CONFIG(release, debug|release){
    DEFINES += SYSTEM_DEVEL #开启开发选项
}else{
   DEFINES += SYSTEM_RELEASE #开启系统发布选项，如开启看门狗
   #DEFINES += QT_NO_DEBUG_OUTPUT #关闭QT调试输出
}

SOURCES += main.cpp\
    WebContextParser.cpp \
    MainWindow.cpp \
    CaiPage.cpp \
    global.cpp \
    DataBase.cpp \
    test.cpp \
    DataService.cpp \
    common.cpp \
    Config.cpp

HEADERS  += \
    WebContextParser.h \
    MainWindow.h \
    CaiPage.h \
    global.h \
    DataBase.h \
    DataService.h \
    common.h \
    Config.h

FORMS    += \
    MainWindow.ui \
    CaiPage.ui

RESOURCES += \
    caip.qrc

OTHER_FILES += \
    log4qt.conf \
    caip.conf \
    readme.txt
