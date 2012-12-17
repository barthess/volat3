#-------------------------------------------------
#
# Project created by QtCreator 2012-12-16T18:12:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tester
TEMPLATE = app


SOURCES += main.cpp \
    tester.cpp

HEADERS  += \
    tester.h

FORMS    += \
    tester.ui

#-----------------------------------------------
# qextserialport shared library
#-----------------------------------------------
INCLUDEPATH += /home/barthess/projects/qextserialport/src
LIBS += /home/barthess/projects/qextserialport-build-Desktop-Release/libqextserialport.so.1.2
# for cross compilation
#LIBS += /home/barthess/projects/qextserialport-build-Som-Release/libqextserialport.so.1.2
DEFINES += QEXTSERIALPORT_USING_SHARED
