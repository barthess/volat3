#ifndef GPSWIDGET_H
#define GPSWIDGET_H

#include <QLabel>

class gpswidget : public QLabel
{
    Q_OBJECT
public:
    explicit gpswidget(QWidget *parent = 0);
    
signals:
    
public slots:
    void update(char *str);
};

#endif // GPSWIDGET_H
