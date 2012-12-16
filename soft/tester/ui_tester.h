/********************************************************************************
** Form generated from reading UI file 'tester.ui'
**
** Created: Sun Dec 16 18:40:23 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTER_H
#define UI_TESTER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Tester
{
public:
    QGroupBox *discreteBox;
    QCheckBox *checkBoxD0;
    QCheckBox *checkBoxD1;
    QCheckBox *checkBoxD2;
    QCheckBox *checkBoxD3;
    QCheckBox *checkBoxD4;
    QCheckBox *checkBoxD5;
    QCheckBox *checkBoxD6;
    QCheckBox *checkBoxD7;
    QGroupBox *analogBox;
    QLabel *labelAn0;
    QProgressBar *barAn1;
    QProgressBar *barAn2;
    QLabel *labelAn1;
    QProgressBar *barAn3;
    QLabel *labelAn2;
    QProgressBar *barAn4;
    QLabel *labelAn3;
    QLabel *labelVbat;
    QLabel *labelTimeUtc;

    void setupUi(QWidget *Tester)
    {
        if (Tester->objectName().isEmpty())
            Tester->setObjectName(QString::fromUtf8("Tester"));
        Tester->resize(410, 200);
        discreteBox = new QGroupBox(Tester);
        discreteBox->setObjectName(QString::fromUtf8("discreteBox"));
        discreteBox->setGeometry(QRect(320, 10, 81, 181));
        checkBoxD0 = new QCheckBox(discreteBox);
        checkBoxD0->setObjectName(QString::fromUtf8("checkBoxD0"));
        checkBoxD0->setEnabled(true);
        checkBoxD0->setGeometry(QRect(10, 20, 61, 20));
        checkBoxD1 = new QCheckBox(discreteBox);
        checkBoxD1->setObjectName(QString::fromUtf8("checkBoxD1"));
        checkBoxD1->setGeometry(QRect(10, 40, 61, 20));
        checkBoxD2 = new QCheckBox(discreteBox);
        checkBoxD2->setObjectName(QString::fromUtf8("checkBoxD2"));
        checkBoxD2->setGeometry(QRect(10, 60, 61, 20));
        checkBoxD3 = new QCheckBox(discreteBox);
        checkBoxD3->setObjectName(QString::fromUtf8("checkBoxD3"));
        checkBoxD3->setGeometry(QRect(10, 80, 61, 20));
        checkBoxD4 = new QCheckBox(discreteBox);
        checkBoxD4->setObjectName(QString::fromUtf8("checkBoxD4"));
        checkBoxD4->setGeometry(QRect(10, 100, 61, 20));
        checkBoxD5 = new QCheckBox(discreteBox);
        checkBoxD5->setObjectName(QString::fromUtf8("checkBoxD5"));
        checkBoxD5->setGeometry(QRect(10, 120, 61, 20));
        checkBoxD6 = new QCheckBox(discreteBox);
        checkBoxD6->setObjectName(QString::fromUtf8("checkBoxD6"));
        checkBoxD6->setGeometry(QRect(10, 140, 61, 20));
        checkBoxD7 = new QCheckBox(discreteBox);
        checkBoxD7->setObjectName(QString::fromUtf8("checkBoxD7"));
        checkBoxD7->setGeometry(QRect(10, 160, 61, 20));
        analogBox = new QGroupBox(Tester);
        analogBox->setObjectName(QString::fromUtf8("analogBox"));
        analogBox->setGeometry(QRect(10, 10, 301, 141));
        labelAn0 = new QLabel(analogBox);
        labelAn0->setObjectName(QString::fromUtf8("labelAn0"));
        labelAn0->setGeometry(QRect(10, 20, 31, 16));
        barAn1 = new QProgressBar(analogBox);
        barAn1->setObjectName(QString::fromUtf8("barAn1"));
        barAn1->setGeometry(QRect(50, 20, 241, 23));
        barAn1->setMaximum(1000);
        barAn1->setValue(240);
        barAn2 = new QProgressBar(analogBox);
        barAn2->setObjectName(QString::fromUtf8("barAn2"));
        barAn2->setGeometry(QRect(50, 50, 241, 23));
        barAn2->setMaximum(1000);
        barAn2->setValue(240);
        labelAn1 = new QLabel(analogBox);
        labelAn1->setObjectName(QString::fromUtf8("labelAn1"));
        labelAn1->setGeometry(QRect(10, 50, 31, 16));
        barAn3 = new QProgressBar(analogBox);
        barAn3->setObjectName(QString::fromUtf8("barAn3"));
        barAn3->setGeometry(QRect(50, 80, 241, 23));
        barAn3->setMaximum(1000);
        barAn3->setValue(240);
        labelAn2 = new QLabel(analogBox);
        labelAn2->setObjectName(QString::fromUtf8("labelAn2"));
        labelAn2->setGeometry(QRect(10, 80, 31, 16));
        barAn4 = new QProgressBar(analogBox);
        barAn4->setObjectName(QString::fromUtf8("barAn4"));
        barAn4->setGeometry(QRect(50, 110, 241, 23));
        barAn4->setMaximum(1000);
        barAn4->setValue(240);
        labelAn3 = new QLabel(analogBox);
        labelAn3->setObjectName(QString::fromUtf8("labelAn3"));
        labelAn3->setGeometry(QRect(10, 110, 31, 16));
        labelVbat = new QLabel(Tester);
        labelVbat->setObjectName(QString::fromUtf8("labelVbat"));
        labelVbat->setGeometry(QRect(20, 180, 111, 16));
        labelTimeUtc = new QLabel(Tester);
        labelTimeUtc->setObjectName(QString::fromUtf8("labelTimeUtc"));
        labelTimeUtc->setGeometry(QRect(20, 160, 281, 16));

        retranslateUi(Tester);

        QMetaObject::connectSlotsByName(Tester);
    } // setupUi

    void retranslateUi(QWidget *Tester)
    {
        Tester->setWindowTitle(QApplication::translate("Tester", "TestWidget", 0, QApplication::UnicodeUTF8));
        discreteBox->setTitle(QApplication::translate("Tester", "Discrete", 0, QApplication::UnicodeUTF8));
        checkBoxD0->setText(QApplication::translate("Tester", "D00", 0, QApplication::UnicodeUTF8));
        checkBoxD1->setText(QApplication::translate("Tester", "D01", 0, QApplication::UnicodeUTF8));
        checkBoxD2->setText(QApplication::translate("Tester", "D02", 0, QApplication::UnicodeUTF8));
        checkBoxD3->setText(QApplication::translate("Tester", "D03", 0, QApplication::UnicodeUTF8));
        checkBoxD4->setText(QApplication::translate("Tester", "D04", 0, QApplication::UnicodeUTF8));
        checkBoxD5->setText(QApplication::translate("Tester", "D05", 0, QApplication::UnicodeUTF8));
        checkBoxD6->setText(QApplication::translate("Tester", "D06", 0, QApplication::UnicodeUTF8));
        checkBoxD7->setText(QApplication::translate("Tester", "D07", 0, QApplication::UnicodeUTF8));
        analogBox->setTitle(QApplication::translate("Tester", "Analog", 0, QApplication::UnicodeUTF8));
        labelAn0->setText(QApplication::translate("Tester", "AN01", 0, QApplication::UnicodeUTF8));
        barAn1->setFormat(QApplication::translate("Tester", "%v", 0, QApplication::UnicodeUTF8));
        barAn2->setFormat(QApplication::translate("Tester", "%v", 0, QApplication::UnicodeUTF8));
        labelAn1->setText(QApplication::translate("Tester", "AN01", 0, QApplication::UnicodeUTF8));
        barAn3->setFormat(QApplication::translate("Tester", "%v", 0, QApplication::UnicodeUTF8));
        labelAn2->setText(QApplication::translate("Tester", "AN01", 0, QApplication::UnicodeUTF8));
        barAn4->setFormat(QApplication::translate("Tester", "%v", 0, QApplication::UnicodeUTF8));
        labelAn3->setText(QApplication::translate("Tester", "AN01", 0, QApplication::UnicodeUTF8));
        labelVbat->setText(QApplication::translate("Tester", "Vbat = ", 0, QApplication::UnicodeUTF8));
        labelTimeUtc->setText(QApplication::translate("Tester", "UTC = ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Tester: public Ui_Tester {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTER_H
