#ifndef WINDOW_CPP
#define WINDOW_CPP

#include "Window.h"
#include <QFont>
#include <QPalette>
#include <QDateTime>
#include <math.h>
#include "../../mavlink/C/oblique/mavlink.h"

Window::Window(QextSerialPort *sp, int timeout, int timezone, QWidget *parent)
{
   setWindowFlags( Qt::FramelessWindowHint );
   setGeometry( 0, 0, 480, 272 );

   QPalette p = this->palette();
   p.setColor( QPalette::Window, Qt::black );
   setPalette( p );

   this->sp       = sp;
   this->timeout  = timeout;
   this->timezone = timezone;

   connect( sp, SIGNAL( readyRead() ), this, SLOT( ReceiveData() ) ); 

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

   FuelLevel    = InitLabel( 270, 140, 22, tr("--%"), Qt::white );
   BatteryLevel = InitLabel( 375, 140, 22, tr("--.-V"), Qt::white );

   bBNAP = false;
   BNAP  = InitLabel( 15, 65, 20, tr("БНАП"), Qt::red );

   bMPIOVD = false;
   MPIOVD  = InitLabel( 15, 95, 20, tr("МПИОВД"), Qt::red );

   bCC = false;
   CC  = InitLabel( 15, 125, 20, tr("ДЦ"), Qt::red );

   Time = InitLabel( 15, 155, 16, tr("--:--:--"), Qt::white );
   Date = InitLabel( 15, 185, 16, tr("--------"), Qt::white );

   N = InitLabel( 15, 15, 14, tr("N:--,-----"), Qt::white );
   E = InitLabel( 15, 35, 14, tr("E:--,-----"), Qt::white );

   H = InitLabel  ( 145, 15, 14, tr("H  :---"), Qt::white );
   Vel = InitLabel( 145, 35, 14, tr("Vel:---"), Qt::white );
   Cog = InitLabel( 145, 55, 14, tr("Cog:---"), Qt::white );
   Fix = InitLabel( 145, 75, 14, tr("Fix:---"), Qt::white );
   Sat = InitLabel( 145, 95, 14, tr("Sat:---"), Qt::white );
   Eph = InitLabel( 145, 115, 14, tr("Eph:---"), Qt::white );
   GSM = InitLabel( 145, 185, 14, tr("GSM:---"), Qt::white );

   Message = new Clickable( "", this );

   f = QFont( "Arial", 16 );
   f.setBold( true );

   Message->setAutoFillBackground( true );

   p = Message->palette();
   p.setColor( QPalette::Window,  Qt::black);
   p.setColor( QPalette::WindowText, QColor( 255, 0, 0 ));

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
   SetColor( BNAP,   bBNAP   ? Qt::green : Qt::red );
   SetColor( MPIOVD, bMPIOVD ? Qt::green : Qt::red );
   SetColor( CC,     bCC     ? Qt::green : Qt::red );

   bBNAP   = false;
   bMPIOVD = false;
   bCC     = false;

   if ( MsgList->count() && ( Message->text() ).isNull() )
   {
      MessageClicked();
   }

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

QLabel *Window::InitLabel( int x, int y, int h, QString text, QColor c )
{
   QLabel *lab = new QLabel( this );
   //QFont     f = QFont( "Courier", h );
   QFont     f = QFont( "Vera", h );
   f.setBold( true );
   QPalette  p = lab->palette();
   
   p.setColor( QPalette::WindowText, c );

   lab->setGeometry( x, y, 0, 0 );
   lab->setPalette( p );
   lab->setFont(f);
   lab->setText(text);
   lab->adjustSize();

   return lab;
}

void Window::SetText( QLabel *lab, QString text )
{
   lab->setText( text );
   lab->adjustSize();
}

void Window::SetColor( QLabel *lab, QColor c )
{
   QPalette p = lab->palette();
   p.setColor( QPalette::WindowText, c );
   lab->setPalette( p );
}

void Window::MessageClicked( void )
{
      Message->setText(QString(""));
}

void Window::CloseClicked( void )
{
   this->close();
}

void Window::ReceiveData( void )
{
   static mavlink_message_t r_message;
   static mavlink_status_t  r_mavlink_status;

   QByteArray ba = sp->readAll();

   for ( int i = 0; i < ba.count(); i++ )
   {
      if ( mavlink_parse_char( 0, ( uint8_t )ba[ i ], &r_message, &r_mavlink_status ) )
      {
         if ( r_message.compid == MAV_COMP_ID_MPIOVD ) bMPIOVD = true;
         if ( r_message.compid == MAV_COMP_ID_BNAP ) bBNAP   = true;
         if ( r_message.compid == MAV_COMP_ID_CC ) bCC     = true;

         mavlink_oblique_rssi_t         *rssi;
         mavlink_global_position_int_t  *gpos;
         mavlink_gps_raw_int_t          *gps_raw;
         mavlink_system_time_t          *sys_time;
         mavlink_mpiovd_sensors_t       *mpiovd_sensors;
         mavlink_statustext_t           *status_text;

         QString s;
         double  d;
         QDateTime dt;
         char buff[60];
         Msg  msg;
    
         switch ( r_message.msgid )
         {
            case  MAVLINK_MSG_ID_OBLIQUE_RSSI:
                  rssi = ( mavlink_oblique_rssi_t * )r_message.payload64;

                  if (rssi->rssi > 31)
                      s = QString("--");
                  else{
                      s = QString::number((rssi->rssi * 2) - 113);
                  }
                  SetText( GSM, QString( "GSM:") + s + QString( "dBm") );
                  break;
            case  MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
                  gpos = ( mavlink_global_position_int_t * )r_message.payload64;

                  s = QString::number( ( double )( gpos->lat / 10000000.0 ), 'f', 5 );
                  SetText( N, (gpos->lon > 0 ? QString( "N:" ) : QString( "S:" )) + s );

                  s = QString::number( ( double )( gpos->lon / 10000000.0 ), 'f', 5 );
                  SetText( E, (gpos->lon > 0 ? QString( "E:" ) : QString( "W:" )) + s );

                  s = QString::number( ( int )( gpos->alt / 1000 ) );
                  SetText( H, QString( "H  :" ) + s );
                  break;
            case  MAVLINK_MSG_ID_GPS_RAW_INT:
                  gps_raw = ( mavlink_gps_raw_int_t * )r_message.payload64;

                  s = QString::number( ( int )( ( 3600 * gps_raw->vel ) / 10000000.0 ) );
                  SetText( Vel, QString( "Vel:" ) + s + QString( " kmh" ) );

                  s = QString::number( ( int )( gps_raw->cog / 100 ) );
                  SetText( Cog, QString( "Cog:" ) + s );

                  switch ( gps_raw->fix_type )
                  {
                     case  0:
                     case  1:
                        s = QString( "no" );
                        break;
                     case  2:
                        s = QString( "2D" );
                        break;
                     case  3:
                        s = QString( "3D" );
                        break;
                  }
                  SetText( Fix, QString( "Fix:" ) + s );
                  SetText( Sat, QString( "Sat:" ) + QString::number( gps_raw->satellites_visible ) );

                  d = gps_raw->eph / 100.0;
                  d = floor( d * 10 ) / 10.0;
                  SetText( Eph, QString( "Eph:" ) + QString::number( d ) );
                  break;
            case  MAVLINK_MSG_ID_SYSTEM_TIME:
                  sys_time = ( mavlink_system_time_t * )r_message.payload64;

                  dt = QDateTime::fromTime_t( 3600 * timezone + sys_time->time_unix_usec / 1000000L );

                  SetText( Time, dt.toString( "hh:mm:ss" ) );
                  SetText( Date, dt.toString( "dd-MM-yyyy" ) );
                  break;
            case  MAVLINK_MSG_ID_MPIOVD_SENSORS:
                  mpiovd_sensors = ( mavlink_mpiovd_sensors_t * )r_message.payload64;

                  if ( bOil != ( ( mpiovd_sensors->relay & 2 ) != 0 ) )
                  {
                     bOil = ( ( mpiovd_sensors->relay & 2 ) != 0 );

                     Oil->setPixmap( bOil ? QPixmap( ":/images/icons/oil_red.png" ) : 
                                            QPixmap( ":/images/icons/oil_gray.png" ) );
                  }

                  if ( bTemp != ( ( mpiovd_sensors->relay & 4 ) != 0 ) )
                  {
                     bTemp = ( ( mpiovd_sensors->relay & 4 ) != 0 );

                     Temp->setPixmap( bTemp ? QPixmap( ":/images/icons/temp_red.png" ) : 
                                              QPixmap( ":/images/icons/temp_gray.png" ) );
                  }
                  
                  if ( bBreak != ( ( mpiovd_sensors->relay & 8 ) != 0 ) )
                  {
                     bBreak = ( ( mpiovd_sensors->relay & 8 ) != 0 );

                     Break->setPixmap( bBreak ? QPixmap( ":/images/icons/break_red.png" ) : 
                                                QPixmap( ":/images/icons/break_gray.png" ) );
                  }

                  if ( bFuel != ( mpiovd_sensors->analog01 > 10 ) )
                  {
                     bFuel = ( mpiovd_sensors->analog01 > 10 );

                     Fuel->setPixmap( bFuel ? QPixmap( ":/images/icons/fuel_green.png" ) : 
                                              QPixmap( ":/images/icons/fuel_red.png" ) );
                  }
                  if (mpiovd_sensors->analog01 > 100)
                      mpiovd_sensors->analog01 = 100;
                  SetText( FuelLevel, QString::number( mpiovd_sensors->analog01 ) + QString( "%" ) );

                  d = mpiovd_sensors->voltage_battery / 1000.0;
                  if ( bBattery != ( d > 11.0 ) )
                  {
                     bBattery = ( d > 11.0 );

                     Battery->setPixmap( bBattery ? QPixmap( ":/images/icons/battery_green.png" ) : 
                                                    QPixmap( ":/images/icons/battery_red.png" ) );
                  }

                  d = floor( d * 10 ) / 10.0;

                  SetText( BatteryLevel, QString::number( d ) + QString( "V" ) );
                  break;
            case  MAVLINK_MSG_ID_STATUSTEXT:
                  status_text = ( mavlink_statustext_t * )r_message.payload64;

                  buff[ 51 ] = 0;
                  strncpy( buff, status_text->text, 50 );

                  msg.severity = status_text->severity;
                  msg.text     = QString( buff );
                  Message->setText( QString::number( msg.severity ) + QString( ": " ) + msg.text );

                  QPalette p = Message->palette();
                  int R = 255 - msg.severity * 36;
                  int G = 255 - R;
                  p.setColor( QPalette::WindowText, QColor( R, G, 0 ) );
                  Message->setPalette( p );
         }
      }
   }
}

#endif // WINDOW_CPP
