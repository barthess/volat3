#ifndef BNAP_UI_H_
#define BNAP_UI_H_

#define gps_led_on()  palSetPad(IOPORT2, PIOB_LED_GPS)
#define gps_led_off() palClearPad(IOPORT2, PIOB_LED_GPS)
#define gsm_led_on()  palSetPad(IOPORT2, PIOB_LED_GSM)
#define gsm_led_off() palClearPad(IOPORT2, PIOB_LED_GSM)

void UiInit(void);

#endif /* BNAP_UI_H_ */
