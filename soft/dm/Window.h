#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QLabel>
#include <QColor>
#include <QPushButton>
#include "Clickable.h"
#include <QTimer>
#include <QList>


typedef struct
{
   int severity;
   QString text;
}  Msg;

class Window : public QMainWindow
{
   Q_OBJECT
public:
   Window( char *comm, int timeout, int baudrate, int timezone, QWidget *parent = 0 );

private:
   char comm[256];
   int timeout;
   int baudrate;
   int timezone;

   void InitWidgets( void );
   QLabel *InitIcon( int x, int y, const char *path );
   QLabel *InitLabel( int x, int y, int h, char *text, QColor c );
   void   SetColor  ( QLabel *lab, QColor c );

   bool   bOil;
   QLabel *Oil;
   
   bool   bBreak;
   QLabel *Break;
   
   bool   bTemp;
   QLabel *Temp;
   
   bool   bFuel;
   QLabel *Fuel;
   QLabel *FuelLevel;

   bool   bBattery;
   QLabel *Battery;
   QLabel *BatteryLevel;

   bool   bBNAP;
   QLabel *BNAP;

   bool   bMPIOVD;
   QLabel *MPIOVD;

   bool   bDC;
   QLabel *DC;

   QLabel *Time;
   QLabel *Date;

   QLabel *N;
   QLabel *E;
   QLabel *H;
   QLabel *GSM;
   QLabel *Vel;
   QLabel *Cog;
   QLabel *Fix;
   QLabel *Sat;
   QLabel *Eph;

   QList<Msg>  *MsgList;
   Clickable   *Message;

   QTimer *Timer;
   QPushButton *btnClose;

public slots:

   void MessageClicked( void );
   void CloseClicked( void );
   void Timeout( void );
};


#endif // WINDOW_H
