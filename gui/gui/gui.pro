#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T15:34:07
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app
CONFIG += c++11

LIBS += -L/usr/local/lib -L/usr/lib -lboost_system -lboost_filesystem -lboost_thread-mt -lboost_random
LIBS += ../../libteasafe.a /usr/lib/libcryptopp.a

INCLUDEPATH += ../../include
QMAKE_CXXFLAGS += -isystem /usr/local/include
QMAKE_CXXFLAGS += -isystem /usr/include

SOURCES += main.cpp \
           MainWindow.cpp \
           LoaderThread.cpp \
           WorkThread.cpp \
           ItemAdder.cpp \
           ContainerBuilderThread.cpp \
           GUICipherCallback.cpp \
           FileWidget.cpp

HEADERS  += MainWindow.h \
            LoaderThread.h \
            TreeItemPathDeriver.h \
            WorkThread.h \
            ItemAdder.h \
            ContainerBuilderThread.h \
            GUICipherCallback.h \
            FileWidget.h

FORMS += MainWindow.ui

RESOURCES += resources.qrc
