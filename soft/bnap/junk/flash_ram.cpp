#include <RTL.h>
#include "sysconfig.h"

#define AT91C_MC_CORRECT_KEY   ((unsigned int) 0x5A << 24)
/// ���������� ���� �������� 
bool EFCWritePage(U16 usPage){
  /// �������� ����������
  AT91C_BASE_AIC->AIC_DCR = AT91C_BASE_AIC->AIC_DCR  | AT91C_AIC_DCR_GMSK;
  /// �������� ������� ������
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG | (AT91C_MC_PAGEN & (usPage << 8));                        
  /// Wait until the end of Command
  while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP) != AT91C_MC_EOP);
  /// Check for Errors
  if (AT91C_BASE_MC->MC_FSR & (AT91C_MC_PROGE | AT91C_MC_LOCKE)) return false;
  /// �������� ����������
  AT91C_BASE_AIC->AIC_DCR = AT91C_BASE_AIC->AIC_DCR  & ~AT91C_AIC_DCR_GMSK;
  return true;
}
//----------------------------------------------------------------------------
