#ifndef SETTINGS_MODEM_H_
#define SETTINGS_MODEM_H_

#define EEPROM_MODEM_MAX_FIELD_LEN    64
#define EEPROM_MODEM_PIN_OFFSET       0
#define EEPROM_MODEM_PIN_SIZE         sizeof("1234")
#define EEPROM_MODEM_APN_OFFSET       (EEPROM_MODEM_PIN_OFFSET + EEPROM_MODEM_PIN_SIZE)
#define EEPROM_MODEM_APN_SIZE         EEPROM_MODEM_MAX_FIELD_LEN
#define EEPROM_MODEM_USER_OFFSET      (EEPROM_MODEM_APN_OFFSET + EEPROM_MODEM_APN_SIZE)
#define EEPROM_MODEM_USER_SIZE        16
#define EEPROM_MODEM_PASS_OFFSET      (EEPROM_MODEM_USER_OFFSET + EEPROM_MODEM_USER_SIZE)
#define EEPROM_MODEM_PASS_SIZE        16
#define EEPROM_MODEM_SERVER_OFFSET    (EEPROM_MODEM_PASS_SIZE + EEPROM_MODEM_PASS_OFFSET)
#define EEPROM_MODEM_SERVER_SIZE      EEPROM_MODEM_MAX_FIELD_LEN
#define EEPROM_MODEM_PORT_OFFSET      (EEPROM_MODEM_SERVER_OFFSET + EEPROM_MODEM_SERVER_SIZE)
#define EEPROM_MODEM_PORT_SIZE        sizeof("65536")
#define EEPROM_MODEM_LISTEN_OFFSET    (EEPROM_MODEM_PORT_OFFSET + EEPROM_MODEM_PORT_SIZE)
#define EEPROM_MODEM_LISTEN_SIZE      sizeof("65536")


#include "eeprom.h"

bool_t read_modem_param(uint8_t *buf, EepromFileStream *f,  size_t maxsize, size_t offset);
void ModemSettingsInit(void);


#endif /* SETTINGS_MODEM_H_ */