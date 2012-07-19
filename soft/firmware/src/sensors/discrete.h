#ifndef DISCRETE_H_
#define DISCRETE_H_


#define z_check_on()    palSetPad(GPIOE, GPIOE_Z_CHECK)
#define z_check_off()   palClearPad(GPIOE, GPIOE_Z_CHECK)


void DiscreteInitLocal(void);

#endif /* DISCRETE_H_ */
