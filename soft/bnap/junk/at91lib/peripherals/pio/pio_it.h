#ifndef PIO_IT_H
#define PIO_IT_H
//------------------------------------------------------------------------------
/// \unit
/// !Purpose
/// 
/// Configuration and handling of interrupts on PIO status changes.
/// 
/// !Usage
/// 
/// -# Configure an status change interrupt on one or more pin(s) with
///    PIO_ConfigureIt.
/// -# Enable & disable interrupts on pins using PIO_EnableIt and
///    PIO_DisableIt.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "pio.h"
#include "..\board.h"

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
#ifdef __cplusplus
  #define EXTERN extern "C"
#else
  #define EXTERN extern
#endif
EXTERN void PIO_InitializeInterrupts(unsigned int priority);
EXTERN void PIO_ConfigureIt(const Pin *pPin, void (*handler)(const Pin *));
EXTERN void PIO_EnableIt(const Pin *pPin);
EXTERN void PIO_DisableIt(const Pin *pPin);
//------------------------------------------------------------------------------
#endif 

