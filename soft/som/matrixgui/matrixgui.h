#ifndef MATRIXGUI_H
#define MATRIXGUI_H

#include <QtGui>
#include "matrixbutton.h"
#include "matrixlabel.h"

class MatrixGui : public QWidget
{
    Q_OBJECT

public:
    MatrixGui(QSettings *config);

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
    void initButtonGrid(QGridLayout *lay, QSettings *config);
};

#endif // MATRIXGUI_H
