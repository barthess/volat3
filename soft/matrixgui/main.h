#ifndef MAIN_H
#define MAIN_H

#include <QtGui>


class KamertonGui : public QWidget
{
    Q_OBJECT

public:
    KamertonGui();

private slots:
    void launchDm(void);
    void launchXterm(void);
    void launchTermDemo(void);

private:
    QProcess *process;
    QPushButton *xtermButton;
    QPushButton *dmButton;
    QPushButton *termDemoButton;
};

#endif // MAIN_H
