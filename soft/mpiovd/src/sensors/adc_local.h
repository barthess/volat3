#include "ch.h"
#include "hal.h"

#include <mavlink.h>
#include <common.h>
#include <mpiovd.h>

#ifndef ADC_LOCAL_H_
#define ADC_LOCAL_H_

void ADCInitLocal(void);
void adc_process(adcsample_t *in, mavlink_mpiovd_sensors_t *raw);

#endif /* ADC_LOCAL_H_ */

