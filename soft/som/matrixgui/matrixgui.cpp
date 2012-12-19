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

    QVBoxLayout *layout = new QVBoxLayout();
    MatrixLabel *header = new MatrixLabel(tr("--== Kamerton ÜberS0ft™ GUI ==--"));
    layout->addWidget(header);
    layout->addLayout(buttonLayout);

    this->setLayout(layout);
}

