#ifndef WINDOW_CPP
#define WINDOW_CPP

#include "Window.h"
#include <QFont>
#include <QPalette>
#include "C/oblique/mavlink.h"

Window::Window( char *comm, int timeout, int baudrate, int timezone, QWidget *parent )
{
   setWindowFlags( Qt::FramelessWindowHint );
   setGeometry( 0, 0, 480, 272 );

   QPalette p = this->palette();
   p.setColor( QPalette::Window, Qt::black );
   setPalette( p );

   strcpy( this->comm, comm );
   this->timeout  = timeout;
   this->baudrate = baudrate;
   this->timezone = timezone;

   MsgList = new QList<Msg>();

   InitWidgets();
}

void Window::InitWidgets( void )
{
   QPalette p;
   QFont    f;

   bOil     = false;
   Oil      = InitIcon( 270, 25, ":/images/icons/oil_gray.png" );
   bTemp    = false;
   Temp     = InitIcon( 350, 25, ":/images/icons/temp_gray.png" );
   bBreak   = false;
   Break    = InitIcon( 410, 25, ":/images/icons/break_gray.png" );
   bFuel    = false;
   Fuel     = InitIcon( 280, 88, ":/images/icons/fuel_red.png" );
   bBattery = false;
   Battery  = InitIcon( 375, 75, ":/images/icons/battery_red.png" );

   FuelLevel    = InitLabel( 270, 140, 22, "--%", Qt::white );
   BatteryLevel = InitLabel( 375, 140, 22, "--.-V", Qt::white );

   bBNAP = false;
   BNAP  = InitLabel( 15, 65, 20, "BNAP", Qt::red );

   bMPIOVD = false;
   MPIOVD  = InitLabel( 15, 95, 20, "MPIOVD", Qt::red );

   bDC = false;
   DC  = InitLabel( 15, 125, 20, "DC", Qt::red );

   Time = InitLabel( 15, 155, 16, "--:--:--", Qt::white );
   Date = InitLabel( 15, 185, 16, "--------", Qt::white );

   N = InitLabel( 15, 15, 14, "N:--,-----", Qt::white );
   E = InitLabel( 15, 35, 14, "E:--,-----", Qt::white );

   H = InitLabel  ( 145, 15, 14, "H  :---", Qt::white );
   GSM = InitLabel( 145, 35, 14, "GSM:---", Qt::white );
   Vel = InitLabel( 145, 55, 14, "Vel:---", Qt::white );
   Cog = InitLabel( 145, 75, 14, "Cog:---", Qt::white );
   Fix = InitLabel( 145, 95, 14, "Fix:---", Qt::white );
   Sat = InitLabel( 145, 115, 14, "Sat:---", Qt::white );
   Eph = InitLabel( 145, 135, 14, "Eph:---", Qt::white );

   Message = new Clickable( "", this );

   f = QFont( "Arial", 16 );
   f.setBold( true );

   Message->setAutoFillBackground( true );

   p = Message->palette();
   p.setColor( QPalette::Window, QColor( 255, 0, 0 ) );
   p.setColor( QPalette::WindowText, Qt::white );

   Message->setAlignment( Qt::AlignCenter );
   Message->setFont( f );
   Message->setPalette( p );
   Message->setGeometry( 10, 225, 460, 30 );

   Timer = new QTimer();

   btnClose = new QPushButton( this );
   btnClose->setGeometry( 430, 180, 40, 40 );
   btnClose->setText( tr( "X" ) );

   connect( Message,  SIGNAL( clicked() ), this, SLOT( MessageClicked() ) );
   connect( Timer,    SIGNAL( timeout() ), this, SLOT( Timeout() ) );
   connect( btnClose, SIGNAL( clicked() ), this, SLOT( CloseClicked( ) ) );
  
   Timer->start( 1000 );
}

void Window::Timeout( void )
{
//   static int i = 0;

   SetColor( BNAP,   bBNAP   ? Qt::green : Qt::red );
   SetColor( MPIOVD, bMPIOVD ? Qt::green : Qt::red );
   SetColor( DC,     bDC     ? Qt::green : Qt::red );

   bBNAP   = false;
   bMPIOVD = false;
   bDC     = false;

   if ( MsgList->count() && ( Message->text() ).isNull() )
   {
      MessageClicked();
   }

//   Msg msg;
//   msg.severity = i % 8;
//   msg.text     = QString( QString::number( i++ ) );
//   MsgList->append( msg );

   Timer->start( timeout );
}

QLabel *Window::InitIcon( int x, int y, const char *path )
{
   QPixmap pix = QPixmap( path );
   QLabel *lab = new QLabel( this );

   lab->setGeometry( x, y, pix.width(), pix.height() );
   lab->setPixmap( pix );

   return lab;
}

QLabel *Window::InitLabel( int x, int y, int h, char *text, QColor c )
{
   QLabel *lab = new QLabel( this );
   QFont     f = QFont( "Courier", h );
   f.setBold( true );
   QPalette  p = lab->palette();
   
   p.setColor( QPalette::WindowText, c );

   lab->setGeometry( x, y, 0, 0 );
   lab->setPalette( p );
   lab->setFont( f );
   lab->setText( tr( text ) );
   lab->adjustSize();

   return lab;
}

void Window::SetColor( QLabel *lab, QColor c )
{
   QPalette p = lab->palette();
   p.setColor( QPalette::WindowText, c );
   lab->setPalette( p );
}

void Window::MessageClicked( void )
{
   if ( MsgList->count() )
   {
      Msg msg = MsgList->takeFirst();

      Message->setText( QString::number( msg.severity ) + QString( ": " ) + msg.text );

      QPalette p = Message->palette();

      if ( msg.severity == 0 )
      {
         p.setColor( QPalette::Window, QColor( 0, 255, 0 ) );
      }
      else
      {
         p.setColor( QPalette::Window, QColor( 255 - ( 7 - msg.severity ) * 30, 0, 0 ) );
      }

      Message->setPalette( p );
   }
}


void Window::CloseClicked( void )
{
   this->close();
}

#endif // WINDOW_CPP
