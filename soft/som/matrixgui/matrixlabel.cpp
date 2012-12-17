#include "matrixlabel.h"

MatrixLabel::MatrixLabel(const QString &text, QWidget *parent) :
    QLabel(parent)
{
    QPalette p;
    QFont    f;

    f = QFont("Vera", 16);
    f.setBold(false);

    this->setAutoFillBackground(true);
    p = this->palette();
    p.setColor(QPalette::Window,  Qt::black);
    p.setColor(QPalette::WindowText, Qt::white);

    this->setAlignment(Qt::AlignCenter);
    this->setFont(f);
    this->setPalette(p);
    this->setMaximumHeight(24);
    this->setText(text);
}
