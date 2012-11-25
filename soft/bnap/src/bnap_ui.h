#ifndef BNAP_UI_H_
#define BNAP_UI_H_

#define gps_led_on()  palSetPad(IOPORT2, 13)
#define gps_led_off() palClearPad(IOPORT2, 13)
#define gsm_led_on()  palSetPad(IOPORT2, 3)
#define gsm_led_off() palClearPad(IOPORT2, 3)

void UiInit(void);

#endif /* BNAP_UI_H_ */
