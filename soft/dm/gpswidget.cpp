#include "gpswidget.h"

gpswidget::gpswidget(QWidget *parent) :
    QLabel(parent)
{
}

void gpswidget::update(char *str){
    this->setText(str);
}
