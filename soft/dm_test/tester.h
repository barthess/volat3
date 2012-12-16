#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>

namespace Ui {
class Tester;
}

class QextSerialPort;

class TestWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TestWidget(QWidget *parent = 0);
    ~TestWidget();
    
private:
    Ui::Tester *ui;
    QextSerialPort *port;

public slots:
   void onDataAvailable(void);
};

#endif // TESTWIDGET_H
