#ifndef MANAGEMENT_H
#define MANAGEMENT_H
//============================================================================
// Модуль управления и тестирования                                    
//============================================================================

#include <RTL.h>
#include "linkpc.h"
#include "SysConfig.h"
//----------------------------------------------------------------------------

class C_Management
{
  friend void Task_Management(void);
private:
public:
  os_mbx_declare(MailBox, MANAGEMENT_MAILBOX_SIZE); 

  C_Management(void) {os_mbx_init(MailBox, sizeof(MailBox));}
  void PC_CommandHandler(C_Msg* pMsg);
};
//----------------------------------------------------------------------------

extern C_Management Management;
extern OS_TID idtManagement;
__task void Task_Management(void);
bool print_text(const char *pstrz);
//----------------------------------------------------------------------------

#endif //#ifndef MANAGEMENT_H
