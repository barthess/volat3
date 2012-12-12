#include "dmface.h"
#include "ui_dmface.h"

DmFace::DmFace(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DmFace)
{
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(mySlot()));
}

DmFace::~DmFace()
{
    delete ui;
}

void DmFace::mySlot(void){

}
