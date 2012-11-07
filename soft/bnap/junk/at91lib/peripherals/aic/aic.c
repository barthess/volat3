//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include "aic.h"

//------------------------------------------------------------------------------
//         Exported functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Configures the interrupt associated with the given source, using the
/// specified mode and interrupt handler.
/// \param source  Interrupt source to configure.
/// \param mode  Triggering mode of the interrupt.
/// \param handler  Interrupt handler function.
//------------------------------------------------------------------------------
void AIC_ConfigureIT(unsigned int source,
                            unsigned int mode,
                            void (*handler)( void ) __irq)
{
    // Disable the interrupt first
    AT91C_BASE_AIC->AIC_IDCR = 1 << source;

    // Configure mode and handler
    AT91C_BASE_AIC->AIC_SMR[source] = mode;
    AT91C_BASE_AIC->AIC_SVR[source] = (unsigned int) handler;

    // Clear interrupt
    AT91C_BASE_AIC->AIC_ICCR = 1 << source;
}

//------------------------------------------------------------------------------
/// Enables interrupts coming from the given (unique) source.
/// \param source  Interrupt source to enable.
//------------------------------------------------------------------------------
void AIC_EnableIT(unsigned int source)
{
    AT91C_BASE_AIC->AIC_IECR = 1 << source;
}

//------------------------------------------------------------------------------
/// Disables interrupts coming from the given (unique) source.
/// \param source  Interrupt source to enable.
//------------------------------------------------------------------------------
void AIC_DisableIT(unsigned int source)
{
    AT91C_BASE_AIC->AIC_IDCR = 1 << source;
}

