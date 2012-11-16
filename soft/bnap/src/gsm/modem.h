#ifndef MODEM_H_
#define MODEM_H_


#define gsm_assert_poweron()  palSetPad(IOPORT1, PIOA_GSM_ON)
#define gsm_release_poweron() palClearPad(IOPORT1, PIOA_GSM_ON)

#define gsm_assert_reset()  palSetPad(IOPORT1, PIOA_GSM_RESET)
#define gsm_release_reset() palClearPad(IOPORT1, PIOA_GSM_RESET)


void ModemInit(void);


#endif /* MODEM_H_ */
