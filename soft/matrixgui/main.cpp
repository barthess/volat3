#include <QtGui>
#include "main.h"

KamertonGui::KamertonGui(){

    xtermButton     = new MatrixButton("://images/icons/sun.png", "Xterm");
    dmButton        = new MatrixButton("://images/icons/kamerton.png", "DM");
    termDemoButton  = new MatrixButton("://images/icons/fuel.png", "TermDemo");
    poweroffButton  = new MatrixButton("://images/icons/q1.png", "Poweroff");
    settingsButton  = new MatrixButton("://images/icons/mosfet.png", "Settings");
    stbButton       = new MatrixButton("://images/icons/stb.png", "Stb");
    chibiButton     = new MatrixButton("://images/icons/chibi.png", "Chibi");
    qualityButton   = new MatrixButton("://images/icons/quality.png", "Quality");

    connect(xtermButton,    SIGNAL(clicked()), this, SLOT(launchXterm()));
    connect(dmButton,       SIGNAL(clicked()), this, SLOT(launchDm()));
    connect(termDemoButton, SIGNAL(clicked()), this, SLOT(launchTermDemo()));
    connect(poweroffButton, SIGNAL(clicked()), this, SLOT(launchPoweroff()));
    connect(settingsButton, SIGNAL(clicked()), this, SLOT(launchSettings()));

    QGridLayout *buttonLayout = new QGridLayout();
    buttonLayout->addWidget(xtermButton,    0, 0);
    buttonLayout->addWidget(stbButton,      0, 1);
    buttonLayout->addWidget(chibiButton,    0, 2);
    buttonLayout->addWidget(settingsButton, 0, 3);
    buttonLayout->addWidget(qualityButton,  1, 0);
    buttonLayout->addWidget(dmButton,       1, 1);
    buttonLayout->addWidget(termDemoButton, 1, 2);
    buttonLayout->addWidget(poweroffButton, 1, 3);

    QVBoxLayout *layout = new QVBoxLayout();
    MatrixLabel *header = new MatrixLabel("--== Kamerton UberS0ft GUI ==--");
    layout->addWidget(header);
    layout->addLayout(buttonLayout);

    this->setLayout(layout);

    process = new QProcess();
}

void KamertonGui::launchXterm(void){
    QString cmd = "xterm";
    this->process->start(cmd);
}

void KamertonGui::launchDm(void){
    QString cmd = "LD_LIBRARY_PATH=/home/root/libs /home/root/dm";
    this->process->start(cmd);
//    QStringList args = (QStringList() << "-d /dev/ttyS0" << "-z 0" << "-t 3000");
//    this->process->start(cmd, args);
//    QString cmd = "/mnt/work/projects/volat3/soft/dm-build-Desktop-Release/dm";
}

void KamertonGui::launchTermDemo(void){
    QString cmd = "LD_LIBRARY_PATH=/home/root/libs /home/root/terminal";
    this->process->start(cmd);
}

void KamertonGui::launchPoweroff(void){
//    QMessageBox *mb = new QMessageBox();
//    mb->setText("Are you sure you want poweroff me?");
//    mb->show();
    this->process->start("poweroff");
}

void KamertonGui::launchSettings(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Settings applet will be here");
    mb->show();
}

void KamertonGui::launchChibi(void){return;}
void KamertonGui::launchStb(void){return;}
void KamertonGui::launchQuality(void){return;}

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
