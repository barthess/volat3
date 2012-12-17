#include <QMessageBox>
#include "matrixbutton.h"

MatrixButton::MatrixButton(const QString& pixpath, const QString& cmd, QWidget *parent):
    QPushButton(parent)
{
    QPalette p;
    p = this->palette();
    p.setColor(QPalette::ButtonText, Qt::white);
    this->setPalette(p);
    this->setFlat(true);

    QPixmap pix = QPixmap(pixpath);
    this->setIcon(pix);
    this->setIconSize(pix.size());
    //this->setMaximumHeight(1000);
    this->cmd = cmd;
    connect(this, SIGNAL(clicked()), this, SLOT(launch()));
}

void MatrixButton::launch(void){
    if (cmd == ""){
        QMessageBox *mb = new QMessageBox();
        mb->setText("Here will be your application");
        mb->show();
    }
    else{
        QProcess *process = new QProcess();
        process->start(this->cmd);
    }
}
