#ifndef DISCRETE_H_
#define DISCRETE_H_


#define sr_sample_on()  palClearPad(GPIOE, GPIOE_SR_SAMPLE)
#define sr_sample_off() palSetPad(GPIOE, GPIOE_SR_SAMPLE)

#define z_check_on()    setGlobalFlag(SPI_SAMPLE_FLAG); palSetPad(GPIOE, GPIOE_Z_CHECK);
#define z_check_off()   palClearPad(GPIOE, GPIOE_Z_CHECK); clearGlobalFlag(SPI_SAMPLE_FLAG);

void rel_normalize(uint32_t *z_on, uint32_t *z_off, uint32_t *out);
void DiscreteInitLocal(void);


#endif /* DISCRETE_H_ */
