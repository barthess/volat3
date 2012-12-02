#ifndef STORAGE_H_
#define STORAGE_H_

/*
 Минимальная порция данных - 1 блок.
 Блок содержит:
 - сигнатуру
 - метку времени (микросекунды с начала эпохи) для быстрого поиска в массиве
 - размер данных
 - данные подготовленные непосредственно к отправке (кодированный мавлинк)
 - свободное пространство
 - контрольную сумму (crc32)
 */
#define RECORD_SIZE                 512 // size of SDCard block in bytes

#define RECORD_SIGNATURE            0xdeadbeef
#define RECORD_SIGNATURE_SIZE       sizeof(uint32_t)
#define RECORD_SIGNATURE_OFFSET     0

#define RECORD_TIMESTAMP_SIZE       sizeof(int64_t)
#define RECORD_TIMESTAMP_OFFSET     (RECORD_SIGNATURE_OFFSET + RECORD_SIGNATURE_SIZE)

#define RECORD_PAYLOAD_SIZE_SIZE    sizeof(uint32_t)
#define RECORD_PAYLOAD_SIZE_OFFSET  (RECORD_TIMESTAMP_OFFSET + RECORD_TIMESTAMP_SIZE)

#define RECORD_PAYLOAD_OFFSET       (RECORD_PAYLOAD_SIZE_OFFSET + RECORD_PAYLOAD_SIZE_SIZE)

#define RECORD_CRC_SIZE             sizeof(uint32_t)
#define RECORD_CRC_OFFSET           (RECORD_SIZE - RECORD_CRC_SIZE)

/* FS buffer size */
#define STORAGE_BUFF_SIZE           RECORD_SIZE

/**
 * @brief   Structure representing storage device using MMC/SD over driver.
 */
typedef struct {
  /**
   * @brief pointer to initialized MMC driver used for IO.
   */
  MMCDriver   *mmcp;
  /**
   * @brief Current block number suitable for writing.
   */
  uint32_t    tip;
  /**
   * @brief Number of used records. It automatically clamps by microsd boundaries.
   */
  uint32_t    used;
  /**
   * @brief Semaphore for mutual access.
   */
  Semaphore   semaphore;
  /**
   * @brief Temporal working buffer.
   */
  void        *buf;
} BnapStorage;


bool_t bnapStorageDoRecord(BnapStorage *bsp);
bool_t bnapStorageGetRecord(BnapStorage *bsp, uint32_t rec);
void bnapStorageMount(BnapStorage *bsp);
void bnapStorageObjectInit(BnapStorage *bsp, MMCDriver *mmcp, void *mmcbuf);
void bnapStorageStart(BnapStorage *bsp, const MMCConfig *config);
bool_t bnapStorageConnect(BnapStorage *bsp);
bool_t bnapStorageDisconnect(BnapStorage *bsp);
void bnapStorageStop(BnapStorage *bsp);
void bnapStoragaAcquire(BnapStorage *bsp);
void bnapStoragaRelease(BnapStorage *bsp);


#endif /* STORAGE_H_ */
