#include <QtGui>
#include "main.h"

KamertonGui::KamertonGui(){

    process = new QProcess();

    xtermButton = new QPushButton("Xterm");
    xtermButton->setMinimumHeight(50);
    connect(xtermButton, SIGNAL(clicked()), this, SLOT(launchXterm()));

    dmButton = new QPushButton("DM");
    dmButton->setMinimumHeight(50);
    connect(dmButton, SIGNAL(clicked()), this, SLOT(launchDm()));

    termDemoButton = new QPushButton("TermDemo");
    termDemoButton->setMinimumHeight(50);
    connect(termDemoButton, SIGNAL(clicked()), this, SLOT(launchTermDemo()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(xtermButton);
    layout->addWidget(dmButton);
    layout->addWidget(termDemoButton);
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
