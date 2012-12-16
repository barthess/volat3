#include "matrixbutton.h"

MatrixButton::MatrixButton(const QString& pixpath, const QString& text, QWidget *parent):
    QPushButton(parent)
{
    QPalette p;

    p = this->palette();
    p.setColor(QPalette::ButtonText, Qt::white);
    this->setPalette(p);

    this->setFlat(true);

    QPixmap pix = QPixmap(pixpath);
    //this->setText(text);
    this->setIcon(pix);
    this->setIconSize(pix.size());
    this->setMaximumHeight(100);
}
