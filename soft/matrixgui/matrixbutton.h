#ifndef MATRIXBUTTON_H
#define MATRIXBUTTON_H

#include <QPushButton>

class MatrixButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MatrixButton(const QString &pixpath, const QString &text, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // MATRIXBUTTON_H
