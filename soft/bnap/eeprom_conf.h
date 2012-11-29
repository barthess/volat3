#ifndef EEPROM_CONF_H_
#define EEPROM_CONF_H_

#define USE_EEPROM_TEST_SUIT    FALSE

#define EEPROM_PAGE_SIZE        32
#define EEPROM_SIZE             8192
#define EEPROM_I2C_ADDR         0b1010000
#define EEPROM_WRITE_TIME_MS    10
#define EEPROM_TX_DEPTH         (EEPROM_PAGE_SIZE + 2)
#define EEPROM_I2CD             I2CD1


#endif /* EEPROM_CONF_H_ */
