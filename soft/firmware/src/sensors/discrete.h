#ifndef DISCRETE_H_
#define DISCRETE_H_


#define sr_sample_on()  palClearPad(GPIOE, GPIOE_SR_SAMPLE)
#define sr_sample_off() palSetPad(GPIOE, GPIOE_SR_SAMPLE)


#define z_check_on()    palSetPad(GPIOE, GPIOE_Z_CHECK)
#define z_check_off()   palClearPad(GPIOE, GPIOE_Z_CHECK)


uint64_t rel_normalize64(uint8_t *rxbuf_z_on, uint8_t *rxbuf_z_off,
                         uint64_t z_mask, uint64_t gnd_mask, uint64_t vcc_mask);
uint32_t rel_normalize32(uint8_t *rxbuf_z_on, uint8_t *rxbuf_z_off,
                         uint32_t z_mask, uint32_t gnd_mask, uint32_t vcc_mask);
void DiscreteInitLocal(void);


#endif /* DISCRETE_H_ */
