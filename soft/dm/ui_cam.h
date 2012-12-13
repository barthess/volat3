/********************************************************************************
** Form generated from reading UI file 'cam.ui'
**
** Created: Thu Dec 13 15:34:57 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAM_H
#define UI_CAM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_camClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *camClass)
    {
        if (camClass->objectName().isEmpty())
            camClass->setObjectName(QString::fromUtf8("camClass"));
        camClass->resize(600, 400);
        menuBar = new QMenuBar(camClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        camClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(camClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        camClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(camClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        camClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(camClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        camClass->setStatusBar(statusBar);

        retranslateUi(camClass);

        QMetaObject::connectSlotsByName(camClass);
    } // setupUi

    void retranslateUi(QMainWindow *camClass)
    {
        camClass->setWindowTitle(QApplication::translate("camClass", "cam", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class camClass: public Ui_camClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAM_H
