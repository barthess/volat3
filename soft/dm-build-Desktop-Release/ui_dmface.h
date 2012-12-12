/********************************************************************************
** Form generated from reading UI file 'dmface.ui'
**
** Created: Mon Dec 10 19:08:48 2012
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DMFACE_H
#define UI_DMFACE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DmFace
{
public:
    QLabel *gps_label;
    QPushButton *pushButton;

    void setupUi(QWidget *DmFace)
    {
        if (DmFace->objectName().isEmpty())
            DmFace->setObjectName(QString::fromUtf8("DmFace"));
        DmFace->resize(480, 272);
        DmFace->setMinimumSize(QSize(480, 272));
        DmFace->setMaximumSize(QSize(480, 272));
        gps_label = new QLabel(DmFace);
        gps_label->setObjectName(QString::fromUtf8("gps_label"));
        gps_label->setGeometry(QRect(10, 0, 57, 14));
        pushButton = new QPushButton(DmFace);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(390, 240, 83, 25));

        retranslateUi(DmFace);

        QMetaObject::connectSlotsByName(DmFace);
    } // setupUi

    void retranslateUi(QWidget *DmFace)
    {
        DmFace->setWindowTitle(QApplication::translate("DmFace", "DmFace", 0, QApplication::UnicodeUTF8));
        gps_label->setText(QApplication::translate("DmFace", "TextLabel", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("DmFace", "PushButton", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DmFace: public Ui_DmFace {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DMFACE_H
