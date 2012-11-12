#ifndef GPS_H
#define GPS_H
//============================================================================
// Драйвер GPS приемника                                     
//============================================================================

#include <RTL.h>
#include "SysConfig.h"
#include "usart.h"
//----------------------------------------------------------------------------

class C_GPS  
{		 
  friend __task void Task_GPS_Rx(void);
protected:
  C_UART *usart;      // Указатель на интерфейс для связи с GPS приемником
  U16 usCountChar;    // Конечное число принятых сиволов
  U16 usCount;        // Использую в качестве текущего счетчика при приеме символов
  char * pcInputBuf;  // Указатель на буфер для приема данных
  char * pcBufOut;    // Указатель на буфер для отправки данных 
public:
  // Принимаем в 0-й  забираем из 1-го, потом принимаем в 1-й забираем из 0-го
  char pcDataBuf_0[GPS_DATA_BUF_SIZE];                                           
  char pcDataBuf_1[GPS_DATA_BUF_SIZE];   
                                          
  C_GPS(C_UART *usart_) : usart(usart_), usCountChar(0), usCount(0), pcInputBuf(pcDataBuf_0), pcBufOut(NULL) {}
  bool GetData(char ch);

  void DiagnosticsTxDisable(void) {usart->boDiagnosticsTx = false;}
  void DiagnosticsRxDisable(void) {usart->boDiagnosticsRx = false;}
  void DiagnosticsTxEnabled(void) {usart->boDiagnosticsTx = true;}
  void DiagnosticsRxEnabled(void) {usart->boDiagnosticsRx = true;}
  bool IsDiagnosticsRx(void) {return usart->boDiagnosticsRx;}

};
//----------------------------------------------------------------------------

extern C_GPS GPS;
extern OS_TID idtGPS_Rx;
extern OS_TID idtGPS_Tx;
__task void Task_GPS_Rx(void);
//----------------------------------------------------------------------------

#endif //#ifndef GPS_H
