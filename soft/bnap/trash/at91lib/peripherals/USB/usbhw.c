/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USBHW.C
 *      Purpose: USB Hardware Layer Module for Atmel AT91SAM7A3
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This file is part of the uVision/ARM development tools.
 *      Copyright (c) 2005-2006 Keil Software. All rights reserved.
 *      This software may only be used under the terms of a valid, current,
 *      end user licence from KEIL for a compatible version of KEIL software
 *      development tools. Nothing else gives you the right to use it.
 *---------------------------------------------------------------------------*/

#include <AT91SAM7A3.H>                     /* AT91SAM7A3 definitions */

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbuser.h"


#pragma diag_suppress 1293


const BYTE  DualBankEP = 0x36;              /* Dual Bank Endpoint Bit Mask */

const DWORD RX_DATA_BK[2] = {
  AT91C_UDP_RX_DATA_BK0,
  AT91C_UDP_RX_DATA_BK1
};


AT91PS_UDP pUDP = AT91C_BASE_UDP;           /* Global UDP Pointer */

BYTE  RxDataBank[USB_EP_NUM];
BYTE  TxDataBank[USB_EP_NUM];


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {

  /* Enables the 48MHz USB Clock UDPCK and System Peripheral USB Clock */
  AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

  /* Global USB Interrupt: Mode and Vector with Highest Priority and Enable */
  AT91C_BASE_AIC->AIC_SMR[AT91C_ID_UDP] = AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE |
                                          AT91C_AIC_PRIOR_HIGHEST;
  AT91C_BASE_AIC->AIC_SVR[AT91C_ID_UDP] = (unsigned long) USB_ISR;
  AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_UDP);

  /* UDP PullUp (USB_DP_PUP): PB1 Pin */
  /*   Configure as Output and Set to disable Pull-up Resistor */
  AT91C_BASE_PIOB->PIO_PER  = AT91C_PIO_PB1;
  AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB1;
  AT91C_BASE_PIOB->PIO_OER  = AT91C_PIO_PB1;
}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (BOOL con) {

  if (con) {
    /* Enable UDP PullUp (USB_DP_PUP) */
    AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB1;
  } else {
    /* Disable UDP PullUp (USB_DP_PUP) */
    AT91C_BASE_PIOB->PIO_SODR = AT91C_PIO_PB1;
  }
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */

void USB_Reset (void) {
  DWORD n;

  /* Global USB Reset */
  pUDP->UDP_GLBSTATE &= ~(AT91C_UDP_FADDEN | AT91C_UDP_CONFG | AT91C_UDP_RMWUPE);
  pUDP->UDP_FADDR     =  AT91C_UDP_FEN;
  pUDP->UDP_ICR       =  0xFFFFFFFF;

  /* Reset & Disable USB Endpoints */
  for (n = 0; n < USB_EP_NUM; n++) {
    pUDP->UDP_CSR[n] = 0;
    RxDataBank[n] = 0;
    TxDataBank[n] = 0;
  }
  pUDP->UDP_RSTEP = 0xFFFFFFFF;
  pUDP->UDP_RSTEP = 0;

  /* Setup USB Interrupts */
  pUDP->UDP_IER = AT91C_UDP_RXSUSP | AT91C_UDP_RXRSM | AT91C_UDP_EXTRSM |
                  AT91C_UDP_SOFINT | (2^USB_EP_NUM - 1);

  /* Setup Control Endpoint 0 */
  pUDP->UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;
}


/*
 *  USB Suspend Function
 *   Called automatically on USB Suspend
 *    Return Value:    None
 */

void USB_Suspend (void) {
  /* Performed by Hardware */
}


/*
 *  USB Resume Function
 *   Called automatically on USB Resume
 *    Return Value:    None
 */

void USB_Resume (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Function
 *   Called automatically on USB Remote Wakeup
 *    Return Value:    None
 */

void USB_WakeUp (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Configuration Function
 *    Parameters:      cfg:   Enable/Disable
 *    Return Value:    None
 */

void USB_WakeUpCfg (BOOL cfg) {

  if (cfg) {
    pUDP->UDP_GLBSTATE |=  AT91C_UDP_RMWUPE;
  } else {
    pUDP->UDP_GLBSTATE &= ~AT91C_UDP_RMWUPE;
  }
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (DWORD adr) {

  pUDP->UDP_FADDR = AT91C_UDP_FEN | adr;
  if (adr) {
    pUDP->UDP_GLBSTATE |=  AT91C_UDP_FADDEN;
  } else {
    pUDP->UDP_GLBSTATE &= ~AT91C_UDP_FADDEN;
  }
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (BOOL cfg) {

  if (cfg) {
    pUDP->UDP_GLBSTATE |=  AT91C_UDP_CONFG;
  } else {
    pUDP->UDP_GLBSTATE &= ~AT91C_UDP_CONFG;
  }
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  DWORD num, dir;

  num = pEPD->bEndpointAddress & 0x0F;
  dir = pEPD->bEndpointAddress & USB_ENDPOINT_DIRECTION_MASK;

  switch (pEPD->bmAttributes & USB_ENDPOINT_TYPE_MASK) {
    case USB_ENDPOINT_TYPE_CONTROL:
      pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_CTRL;
      break;
    case USB_ENDPOINT_TYPE_ISOCHRONOUS:
      if (dir) {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_ISO_IN;
      } else {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_ISO_OUT;
      }
      break;
    case USB_ENDPOINT_TYPE_BULK:
      if (dir) {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_BULK_IN;
      } else {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_BULK_OUT;
      }
      break;
    case USB_ENDPOINT_TYPE_INTERRUPT:
      if (dir) {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_INT_IN;
      } else {
        pUDP->UDP_CSR[num] = AT91C_UDP_EPTYPE_INT_OUT;
      }
      break;
    default:
      pUDP->UDP_CSR[num] = 0;
      break;
  }
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (DWORD dir) {

  if (dir ) {
    pUDP->UDP_CSR[0] |=  AT91C_UDP_DIR;
  } else {
    pUDP->UDP_CSR[0] &= ~AT91C_UDP_DIR;
  }
  pUDP->UDP_CSR[0] &= ~AT91C_UDP_RXSETUP;
}


/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (DWORD EPNum) {
  pUDP->UDP_CSR[EPNum & 0x0F] |=  AT91C_UDP_EPEDS;
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (DWORD EPNum) {
  pUDP->UDP_CSR[EPNum & 0x0F] &= ~AT91C_UDP_EPEDS;
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (DWORD EPNum) {

  EPNum &= 0x0F;
  pUDP->UDP_CSR[EPNum] &= ~(AT91C_UDP_TXCOMP      | AT91C_UDP_RXSETUP      |
                            AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1  |
                            AT91C_UDP_TXPKTRDY    | AT91C_UDP_FORCESTALL   |
                            AT91C_UDP_STALLSENT);
  pUDP->UDP_RSTEP  |=   1 << EPNum;
  pUDP->UDP_RSTEP  &= ~(1 << EPNum);
  RxDataBank[EPNum] =   0;
  TxDataBank[EPNum] =   0;
}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (DWORD EPNum) {
  pUDP->UDP_CSR[EPNum & 0x0F] |=  AT91C_UDP_FORCESTALL;
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (DWORD EPNum) {
  pUDP->UDP_CSR[EPNum & 0x0F] &= ~AT91C_UDP_FORCESTALL;
}


/*
 *  Read USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

DWORD USB_ReadEP (DWORD EPNum, BYTE *pData) {
  DWORD cnt, n;

  EPNum &= 0x0F;
  cnt = (pUDP->UDP_CSR[EPNum] >> 16) & 0x07FF;
  for (n = 0; n < cnt; n++) {
    *pData++ = (BYTE)pUDP->UDP_FDR[EPNum];
  }
  return (cnt);
}


/*
 *  Write USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *                     cnt:   Number of bytes to write
 *    Return Value:    Number of bytes written
 */

DWORD USB_WriteEP (DWORD EPNum, BYTE *pData, DWORD cnt) {
  DWORD n;

  EPNum &= 0x0F;
  if (pUDP->UDP_CSR[EPNum] & AT91C_UDP_TXPKTRDY) {
    if ((DualBankEP & (1 << EPNum)) && (TxDataBank[EPNum] == 0)) {
      TxDataBank[EPNum] = 1;
    } else {
      return (0);
    }
  }
  for (n = 0; n < cnt; n++) {
    pUDP->UDP_FDR[EPNum] = *pData++;
  }
  pUDP->UDP_CSR[EPNum] |= AT91C_UDP_TXPKTRDY;

  return (cnt);
}


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

DWORD USB_GetFrame (void) {
  DWORD val;

  while ((pUDP->UDP_NUM & (AT91C_UDP_FRM_OK | AT91C_UDP_FRM_ERR)) == 0);
  if (pUDP->UDP_NUM & AT91C_UDP_FRM_OK) {
    val = pUDP->UDP_NUM & AT91C_UDP_FRM_NUM;
  } else {
    val = 0xFFFFFFFF;
  }

  return (val);
}



/*
 *  USB Interrupt Service Routine
 */

void USB_ISR (void) __irq {
  DWORD isr, csr, bkm, n;

  while (isr = pUDP->UDP_ISR) {

    /* End of Bus Reset Interrupt */
    if (isr & AT91C_UDP_ENDBUSRES) {
      USB_Reset();
#if   USB_RESET_EVENT
      USB_Reset_Event();
#endif
      pUDP->UDP_ICR = AT91C_UDP_ENDBUSRES;
    }

    /* USB Suspend Interrupt */
    if (isr & AT91C_UDP_RXSUSP) {
      USB_Suspend();
#if   USB_SUSPEND_EVENT
      USB_Suspend_Event();
#endif
      pUDP->UDP_ICR = AT91C_UDP_RXSUSP;
    }

    /* USB Resume Interrupt */
    if (isr & AT91C_UDP_RXRSM) {
      USB_Resume();
#if   USB_RESUME_EVENT
      USB_Resume_Event();
#endif
      pUDP->UDP_ICR = AT91C_UDP_RXRSM;
    }

    /* External Resume Interrupt */
    if (isr & AT91C_UDP_EXTRSM) {
      USB_WakeUp();
#if   USB_WAKEUP_EVENT
      USB_WakeUp_Event();
#endif
      pUDP->UDP_ICR = AT91C_UDP_EXTRSM;
    }

    /* Start of Frame Interrupt */
    if (isr & AT91C_UDP_SOFINT) {
#if USB_SOF_EVENT
      USB_SOF_Event();
#endif
      pUDP->UDP_ICR = AT91C_UDP_SOFINT;
    }

    /* Endpoint Interrupts */
    for (n = 0; n < USB_EP_NUM; n++) {
      if (isr & (1 << n)) {

        csr = pUDP->UDP_CSR[n];

        /* Setup Packet Received Interrupt */
        if (csr & AT91C_UDP_RXSETUP) {
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_SETUP);
          }
          /* Setup Flag is already cleared in USB_DirCtrlEP */
          /* pUDP->UDP_CSR[n] &= ~AT91C_UDP_RXSETUP; */
        }

        /* Data Packet Received Interrupt */
        bkm = RX_DATA_BK[RxDataBank[n]];
        if (csr & bkm) {
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_OUT);
          }
          pUDP->UDP_CSR[n] &= ~bkm;
          if (DualBankEP & (1 << n)) {
            RxDataBank[n] ^= 1;
          }
        }

        /* Data Packet Sent Interrupt */
        if (csr & AT91C_UDP_TXCOMP) {
          pUDP->UDP_CSR[n] &= ~AT91C_UDP_TXCOMP;
          if (TxDataBank[n]) {
            pUDP->UDP_CSR[n] |= AT91C_UDP_TXPKTRDY;
            TxDataBank[n] = 0;
          }
          if (USB_P_EP[n]) {
            USB_P_EP[n](USB_EVT_IN);
          }
        }

        /* STALL Packet Sent Interrupt */
        if (csr & AT91C_UDP_STALLSENT) {
          if ((csr & AT91C_UDP_EPTYPE) == AT91C_UDP_EPTYPE_CTRL) {
            if (USB_P_EP[n]) {
              USB_P_EP[n](USB_EVT_IN_STALL);
/*            USB_P_EP[n](USB_EVT_OUT_STALL); */
            }
          }
          pUDP->UDP_CSR[n] &= ~AT91C_UDP_STALLSENT;
        }

      }
    }

  }

  *AT91C_AIC_EOICR = 0;                     /* End of Interrupt */
}
