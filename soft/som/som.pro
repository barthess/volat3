TEMPLATE = subdirs

SUBDIRS += \
    tester \
    matrixgui \
    telemetry \
    qextserialport \
    hello

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/qextserialport/release/ -lqextserialport
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/qextserialport/debug/ -lqextserialport
else:unix: LIBS += -L$$OUT_PWD/qextserialport/ -lqextserialport

INCLUDEPATH += $$PWD/qextserialport/src
DEPENDPATH += $$PWD/qextserialport
