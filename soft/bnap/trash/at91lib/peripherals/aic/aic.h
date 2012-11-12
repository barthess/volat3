#ifndef AIC_H
#define AIC_H
//------------------------------------------------------------------------------
/// \dir
/// !Purpose
/// 
/// Methods and definitions for configuring interrupts using the Advanced
/// Interrupt Controller (AIC).
/// 
/// !Usage
/// -# Configure an interrupt source using AIC_ConfigureIT
/// -# Enable or disable interrupt generation of a particular source with
///    AIC_EnableIT and AIC_DisableIT.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "..\board.h"

//------------------------------------------------------------------------------
//         Definitions
//------------------------------------------------------------------------------

#ifndef AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL
    /// Redefinition of missing constant.
    #define AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE
#endif

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------
#ifdef __cplusplus
  #define EXTERN extern "C"
#else
  #define EXTERN extern
#endif
EXTERN void AIC_ConfigureIT(unsigned int source, unsigned int mode, void (*handler)( void ) __irq);
EXTERN void AIC_EnableIT(unsigned int source);
EXTERN void AIC_DisableIT(unsigned int source);
//------------------------------------------------------------------------------

#endif //#ifndef AIC_H

