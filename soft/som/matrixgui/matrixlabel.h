#ifndef MATRIXLABEL_H
#define MATRIXLABEL_H

#include <QLabel>

class MatrixLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MatrixLabel(const QString &text, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // MATRIXLABEL_H
