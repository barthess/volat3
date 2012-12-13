#include <QtGui/QApplication>
#include <QTextCodec>
#include <QPlastiqueStyle>
#include "Window.h"
#include <QMessageBox>

int main( int argc, char *argv[] )
{
   QApplication a(argc, argv);
   a.setStyle( new QPlastiqueStyle() );

   QTextCodec *codec = QTextCodec::codecForName( "WINDOWS-1251" );
   QTextCodec::setCodecForTr( codec );

   char comm[256];

   strcpy( comm, "/dev/ttyS0");

   int baudrate = 115200;
   int timezone = 3;
   int timeout  = 3000;

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


      Window *window = new Window( comm, timeout, baudrate, timezone );

      window->show();

      return a.exec();
   }

   catch( int )
   { }

   QMessageBox *box = new QMessageBox();

   box->setText( "Invalid command line" );
   box->exec();
}

