#ifndef RTC1338_H
#define RTC1338_H
//===========================================================================
// Äנאיגונ RTC DC1338                                     
//============================================================================

#include <RTL.h>
#include <time.h>
#include "SysConfig.h"
#include "msg.h"
#include "twi.h"
//----------------------------------------------------------------------------

class C_RTC1338
{
private:
  const U16 usSCLFrequency;
  const U8 ucAddresSlave;
  const U8 ucAddressSizeAndOper;
  U32   uiTimeZone;
public:
  os_mbx_declare (MailBox, RTC1338_MAILBOX_SIZE);
  C_RTC1338(void);
  U32 Read(U8 ucAddress, U8 * pcData, U8 ucSizeData);
  U32 Write(U8 ucAddress, const U8 * pcData, U8 ucSizeData);

  void SetTimeZone(U8 ucTime);
  U8   GetTimeZone(void) {return uiTimeZone/3600;}
  U16  SetTime(time_t t);
  U16  SetTimeFromStr(char *pstrzTime);
  U16  SetTimeFromUTC(float fTime, U32 uiDate);
  U16  TimeToStr(char *pstrz, U16 usSize);
  U16  TimeToUTC(float *pfTime, U32 *puiDate);
  time_t GetTime(void);
};
//----------------------------------------------------------------------------
extern C_RTC1338 RTC1338;
#endif //#ifndef RTC1338_H
