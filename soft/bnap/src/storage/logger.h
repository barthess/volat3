#ifndef LOGGER_H_
#define LOGGER_H_

/*
 Минимальная порция данных - 1 блок.
 Блок содержит:
 - сигнатуру
 - метку времени (микросекунды с начала эпохи) для быстрого поиска в массиве
 - контрольную сумму (crc32)
 */
#define RECORD_SIZE               512 // size of SDCard block in bytes

#define RECORD_SIGNATURE          0xdeadbeef
#define RECORD_SIGNATURE_SIZE     sizeof(uint32_t)
#define RECORD_SIGNATURE_OFFSET   0

#define RECORD_TIMESTAMP_SIZE     sizeof(int64_t)
#define RECORD_TIMESTAMP_OFFSET   RECORD_SIGNATURE_OFFSET + RECORD_SIGNATURE_SIZE

#define RECORD_CRC_SIZE           sizeof(uint32_t)
#define RECORD_CRC_OFFSET         RECORD_SIZE - RECORD_CRC_SIZE

#define STORAGE_SIGNATURE         "Save the planet - kill yourself"
#define STORAGE_SIZE              0x40000 //records

/* FS buffer size */
#define BUFF_SIZE                 RECORD_SIZE


uint8_t* fill_record(void);
bool_t is_crc_valid(uint8_t *buf);


#endif /* LOGGER_H_ */
