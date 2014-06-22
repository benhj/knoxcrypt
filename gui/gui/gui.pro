#-------------------------------------------------
#
# Project created by QtCreator 2014-06-15T15:34:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
TEMPLATE = app

CONFIG += MAC_CONFIG
MAC_CONFIG {
    QMAKE_CXXFLAGS = -std=c++11 -mmacosx-version-min=10.7
    QMAKE_LFLAGS = -std=c++11 -mmacosx-version-min=10.7
}

LIBS += -L"/usr/local/lib" -lboost_system -lboost_filesystem
LIBS += ../../libteasafe.a

INCLUDEPATH += ../../include \
               /usr/local/include

SOURCES += main.cpp\
        MainWindow.cpp \
    LoaderThread.cpp \
    TeaSafeQTreeVisitor.cpp \
    TreeBuilderThread.cpp

HEADERS  += MainWindow.h \
    LoaderThread.h \
    TeaSafeQTreeVisitor.h \
    TreeBuilderThread.h

FORMS    += MainWindow.ui
