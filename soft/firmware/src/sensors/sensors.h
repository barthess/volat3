#ifndef SENSORS_H_
#define SENSORS_H_


/*
 * Обработанные данные.
 */
typedef struct CompensatedData CompensatedData;
struct CompensatedData{
  uint64_t discrete;
  uint16_t analog[16];
};


/*
 * Сырые данные прямо с сенсоров.
 */
typedef struct RawData RawData;
struct RawData{
  uint64_t discrete;

  uint32_t odometer;          /* количество импульсов с одометра */
  uint32_t groundspeed;       /* */
  uint16_t engine_rpm;        /* Тахометр (rpm)*/

  uint16_t analog[16];
//  uint16_t analog02;
//  uint16_t analog03;
//  uint16_t analog04;
//  uint16_t analog05;
//  uint16_t analog06;
//  uint16_t analog07;
//  uint16_t analog08;
//  uint16_t analog09;
//  uint16_t analog10;
//  uint16_t analog11;
//  uint16_t analog12;
//  uint16_t analog13;
//  uint16_t analog14;
//  uint16_t analog15;
//  uint16_t analog16;
};


void SensorsInit(void);

#endif /* SENSORS_H_ */
