LOG4QTSRCPATH = $${PWD}/../src/log4qt-log4qt/src

!cross_compile{
    PROJECT_LIBDIR = $${PWD}/../x86/lib
}else{
    PROJECT_LIBDIR = $${PWD}/../arm/lib
}

TEMPLATE += fakelib
LIBLOG4QT_NAME = $$qtLibraryTarget(Log4Qt)
TEMPLATE -= fakelib

INCLUDEPATH += $$LOG4QTSRCPATH
DEPENDPATH += $$LOG4QTSRCPATH

!log4qt-buildlib{
    DEFINES += LOG4QT_IMPORTS
    LIBS +=  -L$$PROJECT_LIBDIR  -l$$LIBLOG4QT_NAME
}else{
    DEFINES += LOG4QT_EXPORTS
    INCLUDEPATH += $$LOG4QTSRCPATH/helpers \
                   $$LOG4QTSRCPATH/spi \
                   $$LOG4QTSRCPATH/varia
    DEPENDPATH += $$LOG4QTSRCPATH/helpers \
                   $$LOG4QTSRCPATH/spi \
                   $$LOG4QTSRCPATH/varia

    include($$LOG4QTSRCPATH/Log4Qt.pri)
}
