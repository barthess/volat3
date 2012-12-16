#include <QtGui>
#include "main.h"

KamertonGui::KamertonGui(){

    sunButton       = new MatrixButton("://images/icons/sun.png",       "Xterm");
    kamertonButton  = new MatrixButton("://images/icons/kamerton.png",  "DM");
    fuelButton      = new MatrixButton("://images/icons/fuel.png",      "TermDemo");
    q1Button        = new MatrixButton("://images/icons/q1.png",        "Poweroff");
    mosfetButton    = new MatrixButton("://images/icons/mosfet.png",    "Settings");
    stbButton       = new MatrixButton("://images/icons/stb.png",       "Stb");
    chibiButton     = new MatrixButton("://images/icons/chibi.png",     "Chibi");
    qualityButton   = new MatrixButton("://images/icons/quality.png",   "Quality");

    connect(sunButton,      SIGNAL(clicked()), this, SLOT(launchSun()));
    connect(kamertonButton, SIGNAL(clicked()), this, SLOT(launchKamerton()));
    connect(fuelButton,     SIGNAL(clicked()), this, SLOT(launchFuel()));
    connect(q1Button,       SIGNAL(clicked()), this, SLOT(launchQ1()));
    connect(mosfetButton,   SIGNAL(clicked()), this, SLOT(launchMosfet()));
    connect(chibiButton,    SIGNAL(clicked()), this, SLOT(launchChibi()));
    connect(stbButton,      SIGNAL(clicked()), this, SLOT(launchStb()));
    connect(qualityButton,  SIGNAL(clicked()), this, SLOT(launchQuality()));

    QGridLayout *buttonLayout = new QGridLayout();
    buttonLayout->addWidget(sunButton,      0, 0);
    buttonLayout->addWidget(stbButton,      0, 1);
    buttonLayout->addWidget(chibiButton,    0, 2);
    buttonLayout->addWidget(mosfetButton,   0, 3);
    buttonLayout->addWidget(qualityButton,  1, 0);
    buttonLayout->addWidget(kamertonButton, 1, 1);
    buttonLayout->addWidget(fuelButton,     1, 2);
    buttonLayout->addWidget(q1Button,       1, 3);

    QVBoxLayout *layout = new QVBoxLayout();
    MatrixLabel *header = new MatrixLabel("--== Kamerton UberS0ft GUI ==--");
    layout->addWidget(header);
    layout->addLayout(buttonLayout);

    this->setLayout(layout);

    process = new QProcess();
}

void KamertonGui::launchSun(void){
    QString cmd = "/home/root/lightmaps";
    this->process->start(cmd);
}

void KamertonGui::launchKamerton(void){
    QString cmd = "/home/root/dm";
    this->process->start(cmd);
}

void KamertonGui::launchFuel(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Stub");
    mb->show();
}

void KamertonGui::launchQ1(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Are you sure you want poweroff me?");
    mb->setButtonText(0, "Yes");
    mb->exec();
    this->process->start("poweroff");
}

void KamertonGui::launchMosfet(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Settings applet will be here");
    mb->show();
}

void KamertonGui::launchChibi(void){
    QString cmd = "/home/root/qv4l2";
    this->process->start(cmd);
}

void KamertonGui::launchStb(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Stub");
    mb->show();
}

void KamertonGui::launchQuality(void){
    QString cmd = "/home/root/tester";
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
