#include <QtGui>
#include <QSettings>
#include "matrixgui.h"

#define DEFAULT_ROWS_COUNT      2
#define DEFAULT_COLUMNS_COUNT   4

void MatrixGui::initButtonGrid(QGridLayout *lay, QSettings *config){
    bool need_sync = false;
    int rows, columns;

    rows    = config->value("layout/rows",      0).toInt();
    columns = config->value("layout/columns",   0).toInt();
    QString name = config->fileName();

    if ((rows == 0) || (columns == 0)){
        config->setValue("rows",    DEFAULT_ROWS_COUNT);
        config->setValue("columns", DEFAULT_COLUMNS_COUNT);
        rows = DEFAULT_ROWS_COUNT;
        columns = DEFAULT_COLUMNS_COUNT;
        need_sync = true;
    }

    int c = 0;
    QString s;
    while (c < columns){
        int r = 0;
        while(r < rows){
            s = "button" + QString::number(c) + "-" + QString::number(r);

            QString pixpath = config->value(s + "/icon", "").toString();
            if (pixpath == "")
                pixpath = "://images/icons/empty.png";

            QString cmd = config->value(s + "/cmd", "").toString();

            MatrixButton *button = new MatrixButton(pixpath, cmd);
            lay->addWidget(button, r, c);
            r++;
        }
        c++;
    }

    if (need_sync)
        config->sync();
}

MatrixGui::MatrixGui(QSettings *config){
    QPalette p;
    int r,g,b;

    r = config->value("background/red",     0).toInt();
    g = config->value("background/green",   0).toInt();
    b = config->value("background/blue",    0).toInt();

    p.setColor(QPalette::Window, QColor(r, g, b));
    //p.setColor(QPalette::Window, Qt::black);
    this->setPalette(p);

    this->setGeometry(0, 0, 480, 272);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowTitle(QObject::tr("ÜberShell"));

    QGridLayout *buttonLayout = new QGridLayout();
    initButtonGrid(buttonLayout, config);

//    sunButton       = new MatrixButton("://images/icons/sun.png",       "Xterm");
//    kamertonButton  = new MatrixButton("://images/icons/kamerton.png",  "DM");
//    fuelButton      = new MatrixButton("://images/icons/fuel.png",      "TermDemo");
//    q1Button        = new MatrixButton("://images/icons/q1.png",        "Poweroff");
//    mosfetButton    = new MatrixButton("://images/icons/mosfet.png",    "Settings");
//    stbButton       = new MatrixButton("://images/icons/stb.png",       "Stb");
//    chibiButton     = new MatrixButton("://images/icons/chibi.png",     "Chibi");
//    qualityButton   = new MatrixButton("://images/icons/quality.png",   "Quality");

//    connect(sunButton,      SIGNAL(clicked()), this, SLOT(launchSun()));
//    connect(kamertonButton, SIGNAL(clicked()), this, SLOT(launchKamerton()));
//    connect(fuelButton,     SIGNAL(clicked()), this, SLOT(launchFuel()));
//    connect(q1Button,       SIGNAL(clicked()), this, SLOT(launchQ1()));
//    connect(mosfetButton,   SIGNAL(clicked()), this, SLOT(launchMosfet()));
//    connect(chibiButton,    SIGNAL(clicked()), this, SLOT(launchChibi()));
//    connect(stbButton,      SIGNAL(clicked()), this, SLOT(launchStb()));
//    connect(qualityButton,  SIGNAL(clicked()), this, SLOT(launchQuality()));


//    buttonLayout->addWidget(sunButton,      0, 0);
//    buttonLayout->addWidget(stbButton,      0, 1);
//    buttonLayout->addWidget(chibiButton,    0, 2);
//    buttonLayout->addWidget(mosfetButton,   0, 3);
//    buttonLayout->addWidget(qualityButton,  1, 0);
//    buttonLayout->addWidget(kamertonButton, 1, 1);
//    buttonLayout->addWidget(fuelButton,     1, 2);
//    buttonLayout->addWidget(q1Button,       1, 3);

    QVBoxLayout *layout = new QVBoxLayout();
    MatrixLabel *header = new MatrixLabel(tr("--== Kamerton ÜberS0ft™ GUI ==--"));
    layout->addWidget(header);
    layout->addLayout(buttonLayout);

    this->setLayout(layout);

    process = new QProcess();
}

void MatrixGui::launchSun(void){
    QString cmd = "/home/root/lightmaps";
    this->process->start(cmd);
}

void MatrixGui::launchKamerton(void){
    QString cmd = "/home/root/dm";
    this->process->start(cmd);
}

void MatrixGui::launchFuel(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Stub");
    mb->show();
}

void MatrixGui::launchQ1(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Are you sure you want poweroff me?");
    mb->exec();
    this->process->start("poweroff");
}

void MatrixGui::launchMosfet(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Settings applet will be here");
    mb->show();
}

void MatrixGui::launchChibi(void){
    QString cmd = "/home/root/qv4l2";
    this->process->start(cmd);
}

void MatrixGui::launchStb(void){
    QMessageBox *mb = new QMessageBox();
    mb->setText("Stub");
    mb->show();
}

void MatrixGui::launchQuality(void){
    QString cmd = "/home/root/tester";
    this->process->start(cmd);
}
