#include <QtGui/QApplication>
#include <QTextCodec>
#include <QPlastiqueStyle>
#include <QSettings>
#include "Window.h"
#include "qextserialport.h"

#define DEFAULT_PORT_NAME   "/dev/ttyS0"

int main( int argc, char *argv[] )
{
    bool need_config_sync = false;
    int timeout = 3000;
    int timezone = 0;

    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForTr( codec );

    QCoreApplication::setOrganizationName("KamertonUberSoft");
    QCoreApplication::setApplicationName("telemetry");
    QSettings *config = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                      QCoreApplication::organizationName(),
                                      QCoreApplication::applicationName());
    if (!config->contains("port"))
        need_config_sync = true;

    QString portname = config->value("port", DEFAULT_PORT_NAME).toString();
    config->setValue("port", portname);

    if (need_config_sync)
        config->sync();

    QextSerialPort *p = new QextSerialPort(portname);
    p->open(QextSerialPort::ReadWrite);
    p->setBaudRate(BAUD115200);

    HUD2 *window = new HUD2(p, timeout, timezone);

    window->show();
    return a.exec();
}

