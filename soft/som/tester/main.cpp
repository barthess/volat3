#include <QApplication>
#include <QSettings>

#include "tester.h"

#define DEFAULT_PORT_NAME   "/dev/ttyS0"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("KamertonUberSoft");
    QCoreApplication::setApplicationName("tester");
    QSettings *config = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                      QCoreApplication::organizationName(),
                                      QCoreApplication::applicationName());
    QString portname = config->value("port", DEFAULT_PORT_NAME).toString();
    config->setValue("port", portname);
    config->sync();

    QextSerialPort *p = new QextSerialPort(portname);
    p->open(QextSerialPort::ReadWrite);
    p->setBaudRate(BAUD115200);

    TestWidget w(p);
    w.show();

    return a.exec();
}
