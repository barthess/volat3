#ifndef MAIN_H
#define MAIN_H

#include <QtGui>
#include "matrixbutton.h"
#include "matrixlabel.h"

class KamertonGui : public QWidget
{
    Q_OBJECT

public:
    KamertonGui();

private slots:
    void launchKamerton(void);
    void launchSun(void);
    void launchFuel(void);
    void launchQ1(void);
    void launchMosfet(void);
    void launchStb(void);
    void launchChibi(void);
    void launchQuality(void);

private:
    QProcess *process;

    MatrixButton *sunButton;
    MatrixButton *kamertonButton;
    MatrixButton *fuelButton;
    MatrixButton *q1Button;
    MatrixButton *mosfetButton;
    MatrixButton *stbButton;
    MatrixButton *qualityButton;
    MatrixButton *chibiButton;
};

#endif // MAIN_H
