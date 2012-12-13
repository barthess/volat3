#-------------------------------------------------
#
# Project created by QtCreator 2012-12-10T18:23:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dm
TEMPLATE = app


SOURCES += main.cpp \
            Clickable.cpp \
            Window.cpp

HEADERS  += Clickable.h \
            Window.h \

FORMS    += cam.ui

RESOURCES += \
    resources.qrc
