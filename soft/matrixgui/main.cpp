#include <QtGui>
#include "main.h"

#define display_w   480
#define display_h   272

KamertonGui::KamertonGui(){

    process = new QProcess();

    xtermButton = new QPushButton("Xterm");
    xtermButton->setMaximumHeight(display_h);
    connect(xtermButton, SIGNAL(clicked()), this, SLOT(launchXterm()));

    QPixmap pix = QPixmap("://images/icons/temp_red.png");
    dmButton = new QPushButton(pix, "");
    dmButton->setIconSize(pix.size());
    dmButton->setMaximumHeight(display_h);
    connect(dmButton, SIGNAL(clicked()), this, SLOT(launchDm()));

    termDemoButton = new QPushButton("TermDemo");
    termDemoButton->setMaximumHeight(display_h);
    connect(termDemoButton, SIGNAL(clicked()), this, SLOT(launchTermDemo()));

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(xtermButton,      0, 0);
//    layout->addWidget(dmButton,         0, 1);
//    layout->addWidget(termDemoButton,   0, 2);
//    layout->addWidget(xtermButton,      1, 0);
    layout->addWidget(dmButton,         1, 1);
    layout->addWidget(termDemoButton,   1, 2);

    this->setLayout(layout);
}

void KamertonGui::launchXterm(void){
    QString cmd = "xterm";
    this->process->start(cmd);
}

void KamertonGui::launchDm(void){
    //QString cmd = "LD_LIBRARY_PATH=/home/root/libs /home/root/dm";
//    QStringList args = (QStringList() << "-d /dev/ttyS0" << "-z 3" << "-t 3000");
//    this->process->start(cmd, args);
    QString cmd = "/mnt/work/projects/volat3/soft/dm-build-Desktop-Release/dm";
    this->process->start(cmd);
}

void KamertonGui::launchTermDemo(void){
    QString cmd = "LD_LIBRARY_PATH=/home/root/libs /home/root/terminal";
    this->process->start(cmd);
}



int main(int argv, char **args)
{
    QApplication app(argv, args);

    KamertonGui kgui;

    QPalette p;
    p.setColor(QPalette::Window, Qt::black);
    kgui.setPalette(p);

    kgui.setGeometry(0, 0, 480, 272);
    kgui.setWindowFlags(Qt::FramelessWindowHint);
    kgui.setWindowTitle("UberS0ft");
    kgui.show();

    return app.exec();
}
