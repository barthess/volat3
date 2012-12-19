#ifndef MATRIXGUI_H
#define MATRIXGUI_H

#include <QtGui>
#include "matrixbutton.h"
#include "matrixlabel.h"

class MatrixGui : public QWidget
{
    Q_OBJECT

public:
    MatrixGui(QSettings *config);

private slots:

private:
    void initButtonGrid(QGridLayout *lay, QSettings *config);
};

#endif // MATRIXGUI_H
