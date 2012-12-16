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
    void launchDm(void);
    void launchXterm(void);
    void launchTermDemo(void);
    void launchPoweroff(void);
    void launchSettings(void);
    void launchStb(void);
    void launchChibi(void);
    void launchQuality(void);

private:
    QProcess *process;

    MatrixButton *xtermButton;
    MatrixButton *dmButton;
    MatrixButton *termDemoButton;
    MatrixButton *poweroffButton;
    MatrixButton *settingsButton;
    MatrixButton *stbButton;
    MatrixButton *qualityButton;
    MatrixButton *chibiButton;
};

#endif // MAIN_H
