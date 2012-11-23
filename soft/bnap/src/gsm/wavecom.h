#ifndef WAVECOM_H_
#define WAVECOM_H_


#define gsm_assert_poweron()  palSetPad(IOPORT1, PIOA_GSM_ON)
#define gsm_release_poweron() palClearPad(IOPORT1, PIOA_GSM_ON)

#define gsm_release_reset()   palSetPad(IOPORT1, PIOA_GSM_RESET)
#define gsm_assert_reset()    palClearPad(IOPORT1, PIOA_GSM_RESET)


void ModemInit(void);
void UdpSdWrite(SerialDriver *sdp, const uint8_t *bp, size_t n);


#endif /* WAVECOM_H_ */
