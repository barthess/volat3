#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QSettings>
#include "qextserialport.h"

namespace Ui {
class Tester;
}

class QextSerialPort;

class TestWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TestWidget(QextSerialPort *p, QWidget *parent = 0);
    ~TestWidget();
    
private:
    Ui::Tester *ui;
    QextSerialPort *port;

public slots:
   void onDataAvailable(void);

private slots:
   void quit(void);
};

#endif // TESTWIDGET_H
