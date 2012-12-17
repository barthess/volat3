#-------------------------------------------------
#
# Project created by QtCreator 2012-12-10T18:23:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = telemetry
TEMPLATE = app


SOURCES += main.cpp \
            Clickable.cpp \
            Window.cpp \

HEADERS  += Clickable.h \
            Window.h \

FORMS    += cam.ui

RESOURCES += \
    resources.qrc

#-----------------------------------------------
# qextserialport shared library
#-----------------------------------------------
DEPENDPATH += ../qextserialport
INCLUDEPATH += ../qextserialport/src
#LIBS += /home/barthess/projects/qextserialport-build-Desktop-Release/libqextserialport.so.1.2
LIBS += ../qextserialport/libqextserialport.so.1.2
# for cross compilation
#LIBS += /home/barthess/projects/qextserialport-build-Som-Release/libqextserialport.so.1.2
DEFINES += QEXTSERIALPORT_USING_SHARED
