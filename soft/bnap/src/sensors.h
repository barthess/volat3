#ifndef SENSORS_H_
#define SENSORS_H_

/*
 * Обработанные данные.
 */
typedef struct CompensatedData CompensatedData;
struct CompensatedData{
  float     groundspeed_gps;   // Current ground speed by gps (m/s)
};

/*
 * Сырые данные прямо с сенсоров.
 */
typedef struct RawData RawData;
struct RawData{
  int32_t  gps_latitude;
  int32_t  gps_longitude;
  int32_t  gps_altitude;
  int32_t  gps_speed_knots;
  int32_t  gps_course;
  uint8_t  gps_satellites;
};

void SensorsInit(void);

#endif /* SENSORS_H_ */
