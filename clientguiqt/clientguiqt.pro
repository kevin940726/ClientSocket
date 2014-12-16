#-------------------------------------------------
#
# Project created by QtCreator 2014-12-05T19:37:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clientguiqt
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    thread.cpp \
    recving.cpp

HEADERS  += mainwindow.h \
    thread.h \
    recving.h

FORMS    += mainwindow.ui
