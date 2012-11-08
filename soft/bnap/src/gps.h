#include <stdlib.h>
#include "ch.h"
#include "hal.h"

#ifndef GPS_H_
#define GPS_H_

/* divide by this value to obtain degreese */
#define GPS_FIXED_POINT_SCALE 100000.0f

/* From standard: A sentence may contain up to 80 characters plus "$" and CR/LF.
 * так же вычтем первые 5 символов, которые мы не сохраняем в буфер*/
#define GPS_MSG_LEN 75

#define GP_TALKER    (('G' << 8)  + 'P')
#define GGA_SENTENCE (('G' << 16) + ('G' << 8) + 'A')
#define RMC_SENTENCE (('R' << 16) + ('M' << 8) + 'C')

#define GPS_BAUDRATE 9600

//#define gps_reset_assert()
//#define gps_reset_release()

void GPSInit(void);

#endif /* GPS_H_ */
