#-------------------------------------------------
#
# Project created by QtCreator 2014-12-05T19:37:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clientguiqt
TEMPLATE = app

INCLUDEPATH += /usr/local/ssl/include/openssl

LIBS += -L/usr/local/ssl/lib/libssl.a -L/usr/local/ssl/lib/libcrypto.a -lssl -lcrypto -lcurl

SOURCES += main.cpp\
        mainwindow.cpp \
    thread.cpp \
    recving.cpp

HEADERS  += mainwindow.h \
    thread.h \
    recving.h \
    class.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    mycert.pem \
    mykey.pem
