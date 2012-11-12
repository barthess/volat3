#ifndef PIO_H
#define PIO_H
//------------------------------------------------------------------------------

#include "..\board.h"

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------
#ifdef __cplusplus
  #define EXTERN extern "C"
#else
  #define EXTERN extern
#endif
EXTERN unsigned char PIO_Configure(const Pin *list, unsigned int size);
EXTERN void PIO_Set(const Pin *pin );
EXTERN void PIO_Clear(const Pin *pin);
EXTERN unsigned char PIO_Get(const Pin *pin);
EXTERN unsigned int PIO_GetISR(const Pin *pin);
EXTERN unsigned char PIO_GetOutputDataStatus(const Pin *pin);
//------------------------------------------------------------------------------

#endif 
