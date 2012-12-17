#include "tester.h"
#include "ui_tester.h"

#include "/mnt/work/projects/volat3/soft/mavlink/C/oblique/mavlink.h"

TestWidget::TestWidget(QextSerialPort *p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tester)
{
    ui->setupUi(this);

    this->setGeometry(0, 0, 480, 272);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowTitle(tr("Tester"));
    connect(ui->exitButton, SIGNAL(clicked()), this, SLOT(quit()));

    port = p;
    connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
}

TestWidget::~TestWidget()
{
    delete ui;
}

void TestWidget::onDataAvailable(void){
    mavlink_message_t           msg;
    mavlink_status_t            status;
    mavlink_mpiovd_sensors_t    mavlink_mpiovd_sensors_struct;

    QString s;
    double v;
    qint64 d;
    QByteArray data;
    data = port->readAll();

    for (int i = 0; i < data.count(); i++){
        if (mavlink_parse_char(0, (uint8_t)data[i], &msg, &status)){
            if (msg.msgid == MAVLINK_MSG_ID_MPIOVD_SENSORS){
                mavlink_msg_mpiovd_sensors_decode(&msg, &mavlink_mpiovd_sensors_struct);

                v = mavlink_mpiovd_sensors_struct.voltage_battery;
                v /= 1000.0;
                s = QString("VBat = ") + QString::number(v) + QString(" V");
                ui->labelVbat->setText(s);

                d = mavlink_mpiovd_sensors_struct.time_usec;
                s = QString("UTC = ") + QString::number(d) + QString(" uS");
                ui->labelTimeUtc->setText(s);

                ui->barAn1->setValue(mavlink_mpiovd_sensors_struct.analog01);
                ui->barAn2->setValue(mavlink_mpiovd_sensors_struct.analog02);
                ui->barAn3->setValue(mavlink_mpiovd_sensors_struct.analog03);
                ui->barAn4->setValue(mavlink_mpiovd_sensors_struct.analog04);

                d = mavlink_mpiovd_sensors_struct.relay;
                ui->checkBoxD0->setChecked(d & 1);
                ui->checkBoxD1->setChecked(d & 2);
                ui->checkBoxD2->setChecked(d & 4);
                ui->checkBoxD3->setChecked(d & 8);
                ui->checkBoxD4->setChecked(d & 16);
                ui->checkBoxD5->setChecked(d & 32);
                ui->checkBoxD6->setChecked(d & 64);
                ui->checkBoxD7->setChecked(d & 128);
            }
        }
    }
}

void TestWidget::quit(void){
    QApplication::exit(0);
}
