TEMPLATE = subdirs

SUBDIRS += \
    tester \
    matrixgui \
    qextserialport

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/qextserialport/release/ -lqextserialport
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/qextserialport/debug/ -lqextserialport
else:unix: LIBS += -L$$OUT_PWD/qextserialport/ -lqextserialport

INCLUDEPATH += $$PWD/qextserialport/src
DEPENDPATH += $$PWD/qextserialport

#-----------------------------------------------
# qextserialport shared library
#-----------------------------------------------
#INCLUDEPATH += /home/barthess/projects/qextserialport/src
#LIBS += $$PWD/qextserialport-build-Desktop-Release/libqextserialport.so.1.2
# for cross compilation
#LIBS += $$PWD/qextserialport-build-Som-Release/libqextserialport.so.1.2
#DEFINES += QEXTSERIALPORT_USING_SHARED
