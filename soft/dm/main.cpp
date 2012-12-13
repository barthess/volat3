#include <QtGui/QApplication>
#include <QTextCodec>
#include <QPlastiqueStyle>
#include "Window.h"
#include <QMessageBox>

SerialPort *OpenSerial( char *comm, int baudrate )
{
   SerialPort *sp = new SerialPort();

   sp->setPort( QString( comm ) );

   if ( !sp->open( QIODevice::ReadWrite ) )
   {
      return NULL;
   }

   if ( !sp->setRate( baudrate ) )
   {
      return NULL;
   }

   if ( !sp->setDataBits( SerialPort::Data8 ) )
   {
      return NULL;
   }

   if ( !sp->setParity( SerialPort::NoParity ) )
   {
      return NULL;
   }

   if ( !sp->setStopBits( SerialPort::OneStop ) )
   {
      return NULL;
   }

   if ( !sp->setFlowControl( SerialPort::NoFlowControl ) )
   {
      return NULL;
   }

   return sp;
}


int main( int argc, char *argv[] )
{
   QApplication a(argc, argv);
   a.setStyle( new QPlastiqueStyle() );

   //QTextCodec *codec = QTextCodec::codecForName( "WINDOWS-1251" );
   QTextCodec *codec = QTextCodec::codecForName( "utf8" );
   QTextCodec::setCodecForTr( codec );

   char comm[256];

   strcpy( comm, "/dev/ttyO0");

   int baudrate = 115200;
   int timezone = 3;
   int timeout  = 3000;

   QMessageBox *box = new QMessageBox();

   try
   {
      int i = 1;

      while ( --argc > 0 )
      {
         if ( !strcmp( argv[ i ], "-d" ) )
         { 
            strcpy( comm, argv[ i + 1 ] );
         }
         else
         if ( !strcmp( argv[ i ], "-b" ) )
         { 
            baudrate = atoi( argv[ i + 1 ] );

            if ( !baudrate )
            {
               throw 2;
            }
         }
         else
         if ( !strcmp( argv[ i ], "-z" ) )
         { 
            timezone = atoi( argv[ i + 1 ] );
         }
         else
         if ( !strcmp( argv[ i ], "-t" ) )
         {
            timeout = atoi( argv[ i + 1 ] );

            if ( !timeout )
            {
               throw 4;
            }
         }
         else
         {
            throw 0;
         }

         i    += 2;
         argc -= 2;
      }

      SerialPort *sp = OpenSerial( comm, baudrate );

      if ( !sp )
      {
         box->setText( "Comm open error" );
         box->exec();
         return -1;
      }

      Window *window = new Window( sp, timeout, timezone );

      window->show();

      return a.exec();
   }

   catch( int )
   { }

   box->setText( "Invalid command line" );
   box->exec();
   return -1;
}

