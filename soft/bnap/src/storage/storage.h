#ifndef STORAGE_H_
#define STORAGE_H_

/*
 Минимальная порция данных - 1 блок.
 Блок содержит:
 - сигнатуру
 - метку времени (микросекунды с начала эпохи) для быстрого поиска в массиве
 - цепочка размер-данные. Размер uin16_t, данные подготовленные
 непосредственно к отправке (кодированный мавлинк). Нулевой размер означает
 что данных больше нет.
 - свободное пространство
 - контрольную сумму (crc32)
 */
#define RECORD_SIZE                 512 // size of SDCard block in bytes

#define RECORD_SIGNATURE            0xdeadbeef
#define RECORD_SIGNATURE_SIZE       sizeof(uint32_t)
#define RECORD_SIGNATURE_OFFSET     0

#define RECORD_TIMESTAMP_SIZE       sizeof(int64_t)
#define RECORD_TIMESTAMP_OFFSET     (RECORD_SIGNATURE_OFFSET + RECORD_SIGNATURE_SIZE)

#define RECORD_PAYLOAD_OFFSET       (RECORD_TIMESTAMP_OFFSET + RECORD_TIMESTAMP_SIZE)

#define RECORD_CRC_SIZE             sizeof(uint32_t)
#define RECORD_CRC_OFFSET           (RECORD_SIZE - RECORD_CRC_SIZE)

/* FS buffer size */
#define STORAGE_BUFF_SIZE           RECORD_SIZE

/**
 * @brief   Structure representing storage device using MMC/SD over driver.
 */
typedef struct {
  /**
   * @brief last modification time.
   */
  int64_t     mtime;
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
} BnapStorage_t;


bool_t bnapStorageDoRecord(BnapStorage_t *bsp);
bool_t bnapStorageGetRecord(BnapStorage_t *bsp, uint32_t rec);
void bnapStorageMount(BnapStorage_t *bsp);
void bnapStorageObjectInit(BnapStorage_t *bsp, MMCDriver *mmcp, void *mmcbuf);
void bnapStorageStart(BnapStorage_t *bsp, const MMCConfig *config);
bool_t bnapStorageConnect(BnapStorage_t *bsp);
bool_t bnapStorageDisconnect(BnapStorage_t *bsp);
void bnapStorageStop(BnapStorage_t *bsp);
void bnapStorageAcquire(BnapStorage_t *bsp);
void bnapStorageRelease(BnapStorage_t *bsp);
void bnapStorageVoid(BnapStorage_t *bsp);
void bnapStorageWipe(BnapStorage_t *bsp, SerialDriver *sdp);

#endif /* STORAGE_H_ */
