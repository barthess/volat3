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
            Window.cpp \
    QtAddOnSerialPort/ttylocker_unix.cpp \
    QtAddOnSerialPort/serialportinfo.cpp \
    QtAddOnSerialPort/serialportinfo_unix.cpp \
    QtAddOnSerialPort/serialport.cpp \
    QtAddOnSerialPort/serialport_unix.cpp \

HEADERS  += Clickable.h \
            Window.h \
    QtAddOnSerialPort/ttylocker_unix_p.h \
    QtAddOnSerialPort/serialportinfo.h \
    QtAddOnSerialPort/serialportinfo_p.h \
    QtAddOnSerialPort/serialport.h \
    QtAddOnSerialPort/serialport-global.h \
    QtAddOnSerialPort/serialport_unix_p.h \
    QtAddOnSerialPort/serialport_p.h \
    QtAddOnSerialPort/qt4support/qringbuffer_p.h

FORMS    += cam.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    QtAddOnSerialPort/qt4support/serialport.prf \
    QtAddOnSerialPort/qt4support/install-helper.pri
