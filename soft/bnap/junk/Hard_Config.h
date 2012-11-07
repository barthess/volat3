#ifndef HARDCONFIG_H
#define HARDCONFIG_H
//============================================================================
// TINS5                                    
//============================================================================

#include <RTL.h>
#include <AT91SAM7A3.H>

//*** <<< Use Configuration Wizard in Context Menu >>> ***
#ifndef OS_TASKCNT
  #define KBDB_7 AT91C_PIO_PB21
#endif

//   <o0> FMCN: Flash Microsecond Cycle Number  <0-40000000>
#define MCK 184322
//EFC1_FMR_Val1    EQU     30000000

// <e> Embedded Flash Controller 1 (EFC1)
//   <o1.0..15> FMCN: Flash Microsecond Cycle Number 
// </e>
//EFC1_SETUP      EQU     1
//EFC1_FMR_Val    EQU     55

//----------------------------------------------------------------------------

//*** <<< end of configuration section >>>    ***

#endif
