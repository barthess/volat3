#ifndef CLICKABLE_H
#define CLICKABLE_H

#include <QObject>
#include <QLabel>

class Clickable : public QLabel
{
   Q_OBJECT
public:
   Clickable( const QString& text, QWidget *parent = 0 );

signals:
   void clicked();

protected:
   void mousePressEvent( QMouseEvent *ev ) ;

};

#endif // CLICKABLE_H