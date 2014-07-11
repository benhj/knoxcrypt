#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T15:34:07
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app

macx {
    CONFIG += MAC_CONFIG
    MAC_CONFIG {
        QMAKE_CXXFLAGS = -std=c++11 -mmacosx-version-min=10.7
        QMAKE_LFLAGS = -std=c++11 -mmacosx-version-min=10.7
    }
}

unix {
    CONFIG += UNIX_CONFIG
    UNIX_CONFIG {
        QMAKE_CXXFLAGS = -std=c++11
        QMAKE_LFLAGS = -std=c++11
    }
}

LIBS += -L"/usr/local/lib" -lboost_system -lboost_filesystem -lboost_thread-mt -lboost_random
LIBS += ../../libteasafe.a

INCLUDEPATH += ../../include \
               /usr/local/include

SOURCES += main.cpp \
           MainWindow.cpp \
           LoaderThread.cpp \
           WorkThread.cpp \
    ItemAdder.cpp \
    ContainerBuilderThread.cpp \
    GUICipherCallback.cpp

HEADERS  += MainWindow.h \
            LoaderThread.h \
            TreeItemPathDeriver.h \
            WorkThread.h \
    ItemAdder.h \
    ContainerBuilderThread.h \
    GUICipherCallback.h

FORMS += MainWindow.ui

RESOURCES += \
    resources.qrc
