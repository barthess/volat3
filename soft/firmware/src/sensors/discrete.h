#ifndef DISCRETE_H_
#define DISCRETE_H_


#define sr_sample_on()  palClearPad(GPIOE, GPIOE_SR_SAMPLE)
#define sr_sample_off() palSetPad(GPIOE, GPIOE_SR_SAMPLE)


#define z_check_on()    palSetPad(GPIOE, GPIOE_Z_CHECK)
#define z_check_off()   palClearPad(GPIOE, GPIOE_Z_CHECK)


void DiscreteInitLocal(void);

#endif /* DISCRETE_H_ */
