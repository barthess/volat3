#ifndef CLICKABLE_CPP
#define CLICKABLE_CPP

#include "Clickable.h"

Clickable::Clickable( const QString& text, QWidget *parent )
         : QLabel( parent )
{ 
   setText( text );
}

void Clickable::mousePressEvent( QMouseEvent *ev )
{
   emit clicked();
}

#endif // CLICKABLE_CPP