#ifndef MAIN_H
#define MAIN_H
//============================================================================
// TINS5                                    
//============================================================================
//*** <<< Use Configuration Wizard in Context Menu >>> ***

//   <o0> FMCN: Flash Microsecond Cycle Number  <0-40000000>
#define MCK 184322
//EFC1_FMR_Val1    EQU     30000000

;// <e> Embedded Flash Controller 1 (EFC1)
;//   <o1.0..15> FMCN: Flash Microsecond Cycle Number 
;// </e>
EFC1_SETUP      EQU     1
EFC1_FMR_Val    EQU     55

//----------------------------------------------------------------------------
iyuiuyi
//*** <<< end of configuration section >>>    ***

#endif
