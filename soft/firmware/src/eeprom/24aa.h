/*
Copyright 2012 Uladzimir Pylinski aka barthess.
You may use this work without restrictions, as long as this notice is included.
The work is provided "as is" without warranty of any kind, neither express nor implied.
*/

#ifndef _24AA_H_
#define _24AA_H_

/* page size in bytes. Consult datasheet. */
#define EEPROM_PAGE_SIZE  32

/* total amount of memory in bytes */
#define EEPROM_SIZE       4096

/* ChibiOS I2C driver used to communicate with EEPROM */
#define EEPROM_I2CD       I2CD1

/* EEPROM address on bus */
#define EEPROM_I2C_ADDR   0b1010000

/* time to write one page in mS. Consult datasheet! */
#define EEPROM_WRITE_TIME 5

/* temporal transmit buffer depth for eeprom driver */
#define EEPROM_TX_DEPTH   (EEPROM_PAGE_SIZE + 2)

/* mutual exclusion switch */
#define EEPROM_USE_MUTUAL_EXCLUSION   TRUE

void init_eepromio(void);
msg_t eeprom_read(uint32_t offset, uint8_t *buf, size_t len);
msg_t eeprom_write(uint32_t offset, const uint8_t *buf, size_t len);

#endif /* _24AA_H_ */



