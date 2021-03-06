#include <math.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "mavlink.h"

#include "main.h"
#include "sensors.h"
#include "gps.h"
#include "message.h"
#include "bnap_ui.h"

/**
 * Широта  — это угол между отвесной линией в данной точке и плоскостью экватора,
отсчитываемый от 0 до 90° в обе стороны от экватора. Географическую широту
точек, лежащих в северном полушарии, (северная широта) принято считать
положительной, широту точек в южном полушарии — отрицательной.
 * Долгота — угол между плоскостью меридиана, проходящего через данную точку,
и плоскостью начального нулевого меридиана, от которого ведётся отсчёт долготы.
Долготы от 0° до 180° к востоку от нулевого меридиана называют восточными,
к западу — западными. Восточные долготы принято считать положительными,
западные — отрицательными.
*/

/**
A sentence may contain up to 80 characters plus "$" and CR/LF.
If data for a field is not available, the
field is omitted, but the delimiting commas are still sent,
with no space between them. The checksum
field consists of a "*" and two hex digits representing the exclusive
OR of all characters between, but not
including, the "$" and "*".
*/

/**
 * Integral values
 * Values returned by the core TinyGPS methods are integral. Angular
 * latitude and longitude measurements, for example, are provided in units
 * of 10^(-5) degrees, so instead of 90°30'00", get_position() returns a
 * longitude value of 9050000, or 90.5 degrees.
 *
 * Высота по алгоритму получается в сантиметрах
 */

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define COG_UNKNOWN     65535
#define VEL_UNKNOWN     65535
#define HDG_UNKNOWN     65535

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern Thread *gps_tp;
extern GlobalFlags_t GlobalFlags;
extern RawData raw_data;
extern struct tm gps_timp;

extern mavlink_gps_raw_int_t mavlink_gps_raw_int_struct;
extern mavlink_global_position_int_t mavlink_global_position_int_struct;

extern EventSource event_mavlink_gps_raw_int;
extern EventSource event_mavlink_global_position_int;
extern EventSource event_gps_time_got;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static const SerialConfig gps_ser_cfg = {
    GPS_BAUDRATE,
    AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS |
                               AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
};

/**/
static uint8_t ggabuf[GPS_MSG_LEN];
static uint8_t rmcbuf[GPS_MSG_LEN];

/* checksum thumbnalis */
static uint8_t ggachecksum = 'G' ^ 'P' ^ 'G' ^ 'G' ^ 'A';
static uint8_t rmcchecksum = 'G' ^ 'P' ^ 'R' ^ 'M' ^ 'C';

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
static void parse_rmc(uint8_t *rmcbuf);
static void parse_gga(uint8_t *ggabuf);
static void gps_get_time(struct tm *timp, uint8_t *buft, uint8_t *bufd);
static int32_t parse_decimal(uint8_t *p);
static int32_t parse_degrees(uint8_t *p);
static uint32_t gpsatol(const uint8_t *str);
static bool_t gpsisdigit(char c);
static uint8_t get_gps_sentence(uint8_t *buf, uint8_t checksum, SerialDriver *sdp);
static uint8_t from_hex(uint8_t a);

/*
 *******************************************************************************
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 *******************************************************************************
 */
/**
 *
 */
static void __init_mavlink_structs(void){
  /* default values (no good coordinates) */
  mavlink_gps_raw_int_struct.alt = 0;
  mavlink_gps_raw_int_struct.cog = 0;
  mavlink_gps_raw_int_struct.eph = 0;
  mavlink_gps_raw_int_struct.epv = 0;
  mavlink_gps_raw_int_struct.fix_type = 0;
  mavlink_gps_raw_int_struct.lat = 0;
  mavlink_gps_raw_int_struct.lon = 0;
  mavlink_gps_raw_int_struct.satellites_visible = 0;
  mavlink_gps_raw_int_struct.time_usec = 0;
  mavlink_gps_raw_int_struct.vel = 0;

  mavlink_global_position_int_struct.alt = 0;
  mavlink_global_position_int_struct.hdg = HDG_UNKNOWN;
  mavlink_global_position_int_struct.lat = 0;
  mavlink_global_position_int_struct.lon = 0;
  mavlink_global_position_int_struct.relative_alt = 0;
  mavlink_global_position_int_struct.time_boot_ms = TIME_BOOT_MS;
  mavlink_global_position_int_struct.vx = 0;
  mavlink_global_position_int_struct.vy = 0;
  mavlink_global_position_int_struct.vz = 0;
}

/**
 *
 */
static WORKING_AREA(gpsRxThreadWA, 256);
static msg_t gpsRxThread(void *arg){
  chRegSetThreadName("gpsRx");
  SerialDriver *sdp = arg;
  uint32_t tmp = 0;
  uint32_t n = 0;

  __init_mavlink_structs();

  while(!(chThdShouldTerminate())){
    if (n >= 2){
      chEvtBroadcastFlags(&event_mavlink_gps_raw_int, EVMSK_MAVLINK_GPS_RAW_INT);
      mavlink_global_position_int_struct.time_boot_ms = TIME_BOOT_MS;
      chEvtBroadcastFlags(&event_mavlink_global_position_int, EVMSK_MAVLINK_GLOBAL_POSITION_INT);
      n = 0;
    }

		tmp = 0;
		while(sdGet(sdp) != '$')
			; // читаем из буфера до тех пор, пока не найдем знак бакса

		tmp = sdGet(sdp) << 8;
		tmp = tmp + sdGet(sdp);
		if (tmp != GP_TALKER)
			continue;

		// определим тип сообщения
		tmp = sdGet(sdp) << 16;
		tmp = tmp + (sdGet(sdp) << 8);
		tmp = tmp + sdGet(sdp);
		if (tmp == GGA_SENTENCE){
	    if (get_gps_sentence(ggabuf, ggachecksum, sdp) == 0){
	      parse_gga(ggabuf);
	      n++;
	    }
	    continue;
		}
		if (tmp == RMC_SENTENCE){
	    if (get_gps_sentence(rmcbuf, rmcchecksum, sdp) == 0){
	      parse_rmc(rmcbuf);
	      n++;
	    }
	    continue;
		}
		else
			continue;
  }

  chThdExit(0);
  return 0;
}

/*
$GPGGA,115436.000,5354.713670,N,02725.690517,E,1,5,2.01,266.711,M,26.294,M,,*5D
$GPGGA,000103.037,,,,,0,0,,,M,,M,,*4E

$GPRMC,115436.000,A,5354.713670,N,02725.690517,E,0.20,210.43,010611,,,A*66
$GPRMC,115436.000,,,,,,0.20,210.43,010611,,,A*66
$GPRMC,115436.000,,,,,,,,,,,A*66
*/
static void parse_gga(uint8_t *ggabuf){
  // для широты и долготы выбран знаковый формат чтобы не таскать N, S, W, E
  int32_t  gps_latitude = 0;
  int32_t  gps_longitude = 0;
  int32_t  gps_altitude = 0;
  int32_t  hdop = 0;

  uint8_t i = 1; /* начинается с 1, потому что нулевым символом является рудиментарная запятая */
  uint8_t fix = 0, satellites_visible = 0;

  while(ggabuf[i] != ','){i++;}                 /* time */
  i++;

  gps_latitude = parse_degrees(&ggabuf[i]);     /* latitude */
  while(ggabuf[i] != ','){i++;}
    i++;

  if (ggabuf[i] == 'S')                         /* latitude letter */
    gps_latitude = -abs(gps_latitude);
  while(ggabuf[i] != ','){i++;}
    i++;

  gps_longitude = parse_degrees(&ggabuf[i]);    /* longitude */
  while(ggabuf[i] != ','){i++;}
    i++;

	if (ggabuf[i] == 'W')
	  gps_longitude = -abs(gps_longitude);        /* longitude letter parse */
  while(ggabuf[i] != ','){i++;}
    i++;

  fix = ggabuf[i] - '0';                        /* gps quallity */
  while(ggabuf[i] != ','){i++;}
    i++;

  satellites_visible = gpsatol(&ggabuf[i]);     /* number of satellites */
  while(ggabuf[i] != ','){i++;}
    i++;

  hdop = parse_decimal(&ggabuf[i]);
  while(ggabuf[i] != ','){i++;}                 /* Horizontal Dilution of precision */
    i++;

  gps_altitude = parse_decimal(&ggabuf[i]);     /* altitude */
  while(ggabuf[i] != ','){i++;}
    i++;

  while(ggabuf[i] != ','){i++;}                 /* units of altitude */
    i++;

  while(ggabuf[i] != ','){i++;}                 /* geoidal separation */
    i++;

	if (GlobalFlags.gps_valid == 1){  /* если есть достоверные координаты */
    raw_data.gps_latitude   = gps_latitude;
    raw_data.gps_longitude  = gps_longitude;
    raw_data.gps_altitude   = gps_altitude;
    raw_data.gps_satellites = satellites_visible;

    mavlink_gps_raw_int_struct.lat = gps_latitude   * 100;
    mavlink_gps_raw_int_struct.lon = gps_longitude  * 100;
    mavlink_gps_raw_int_struct.alt = gps_altitude   * 10;
    mavlink_gps_raw_int_struct.eph = hdop;

    mavlink_global_position_int_struct.lat = mavlink_gps_raw_int_struct.lat;
    mavlink_global_position_int_struct.lon = mavlink_gps_raw_int_struct.lon;
    mavlink_global_position_int_struct.alt = mavlink_gps_raw_int_struct.alt;
	}
	else{
    raw_data.gps_latitude = 0;
    raw_data.gps_longitude = 0;
    raw_data.gps_altitude = 0;
    raw_data.gps_satellites = 0;

    mavlink_gps_raw_int_struct.lat = 0;
    mavlink_gps_raw_int_struct.lon = 0;
    mavlink_gps_raw_int_struct.alt = 0;
    mavlink_gps_raw_int_struct.eph = 0;

    mavlink_global_position_int_struct.lat = 0;
    mavlink_global_position_int_struct.lon = 0;
    mavlink_global_position_int_struct.alt = 0;
	}
	mavlink_gps_raw_int_struct.satellites_visible = satellites_visible;
	mavlink_gps_raw_int_struct.fix_type = fix + 1; /* +1 to conform with mavlink rules */
}

static void parse_rmc(uint8_t *rmcbuf){
  int32_t  gps_speed_knots = 0;
  int32_t  gps_course = 0;

  uint8_t *buft, *bufd;
  uint8_t i = 1;  /* начинается с 1, потому что нулевым символом является рудиментарная запятая */
  uint8_t valid = 'V';

  buft = &(rmcbuf[i]);
  while(rmcbuf[i] != ','){i++;}                   /* Time (UTC) */
    i++;

  valid = rmcbuf[i];                              /* Status, V = Navigation receiver warning */
  while(rmcbuf[i] != ','){i++;}
    i++;

  while(rmcbuf[i] != ','){i++;}                   /* Latitude */
    i++;

  while(rmcbuf[i] != ','){i++;}                   /* N or S */
    i++;

  while(rmcbuf[i] != ','){i++;}                   /* Longitude */
    i++;

  while(rmcbuf[i] != ','){i++;}                   /* E or W */
    i++;

  gps_speed_knots = parse_decimal(&rmcbuf[i]);    /*Speed over ground, knots */
  while(rmcbuf[i] != ','){i++;}
    i++;

  gps_course = parse_decimal(&rmcbuf[i]);         /*Course Over Ground */
  while(rmcbuf[i] != ','){i++;}
    i++;

  bufd = &(rmcbuf[i]);
  while(rmcbuf[i] != ','){i++;}                   /* Date (UTC) */
    i++;

  if (valid == 'A'){                              /* если координаты достоверны */
  	raw_data.gps_course      = gps_course;
  	raw_data.gps_speed_knots = gps_speed_knots;
  	setGlobalFlag(GlobalFlags.gps_valid);
    gps_get_time(&gps_timp, buft, bufd);
    mavlink_gps_raw_int_struct.cog = gps_course;
    mavlink_gps_raw_int_struct.vel = gps_speed_knots * 51;
  }
  else{
    clearGlobalFlag(GlobalFlags.gps_valid);
  	raw_data.gps_course = 0;
  	raw_data.gps_speed_knots = 0;
  	mavlink_gps_raw_int_struct.cog = 0;
  	mavlink_gps_raw_int_struct.vel = 0;
  }
}

/**
 * Выковыривает дату и время из RMC
 *
 * timp - указатель на структуру с временем
 * buft - указатель на строку с временем
 * bufd - указатель на строку с датой
 *
int tm_sec       seconds after minute [0-61] (61 allows for 2 leap-seconds)
int tm_min       minutes after hour [0-59]
int tm_hour      hours after midnight [0-23]
int tm_mday      day of the month [1-31]
int tm_mon       month of year [0-11]
int tm_year      current year-1900
int tm_wday      days since Sunday [0-6]
int tm_yday      days since January 1st [0-365]
int tm_isdst     daylight savings indicator (1 = yes, 0 = no, -1 = unknown)
 */
static void gps_get_time(struct tm *timp, uint8_t *buft, uint8_t *bufd){
  timp->tm_hour = 10 * (buft[0] - '0') + (buft[1] - '0');
  timp->tm_min  = 10 * (buft[2] - '0') + (buft[3] - '0');
  timp->tm_sec  = 10 * (buft[4] - '0') + (buft[5] - '0');

  timp->tm_mday = 10 * (bufd[0] - '0') + (bufd[1] - '0');
  timp->tm_mon  = 10 * (bufd[2] - '0') + (bufd[3] - '0') - 1;
  timp->tm_year = 10 * (bufd[4] - '0') + (bufd[5] - '0') + 2000 - 1900;

  chEvtBroadcastFlags(&event_gps_time_got, EVMSK_GPS_TIME_GOT);
}

/**
 *
 */
static uint8_t get_gps_sentence(uint8_t *buf, uint8_t checksum, SerialDriver *sdp){
  uint8_t byte = 0, i = 0;

  while TRUE{
    i++;
    if (i >= GPS_MSG_LEN)   /* если данных больше, чем поместится в буфер длинной len */
      return 1;
    byte = sdGet(sdp);
    if (byte == '*')        /* как только натыкаемся на * - выходим из цикла */
      break;
    checksum ^= byte;
    *buf++ = byte;
  }
  checksum ^= from_hex(sdGet(sdp)) * 16; /* читаем 2 байта контрольной суммы */
  checksum ^= from_hex(sdGet(sdp));

  if(checksum == 0)    /* сошлась */
    return 0;
  else
    return 2;
}

static uint8_t from_hex(uint8_t a){
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}

/**
 * Возвращает значение с фиксированной точкой с точностью 2 знака после запятой
 */
static int32_t parse_decimal(uint8_t *p){
  bool_t isneg = (*p == '-'); /* обработаем наличие знака "-" */
  if (isneg) ++p;
  uint32_t ret = gpsatol(p);  /* сделаем заготовку для возвращаемого значения */
  ret = ret * 100UL;          /* сделаем место для 2 знаков после запятой */

  while (gpsisdigit(*p)) ++p; /* пропустим все знаки до запятой - мы их уже обработали */
  if (*p == '.'){             /* запишем 2 знака после запятой */
    if (gpsisdigit(p[1])){
      ret += 10 * (p[1] - '0');
      if (gpsisdigit(p[2]))
        ret += p[2] - '0';
    }
  }
  return isneg ? -ret : ret;
}

static int32_t parse_degrees(uint8_t *p){
  uint32_t left = gpsatol(p);                       /* читаем первую часть (ddmm) */
  uint32_t tenk_minutes = (left % 100UL) * 10000UL; /* отделяем целые части минут */

  while (gpsisdigit(*p)) ++p;
  if (*p == '.'){
    uint32_t mult = 1000; /* только 3 знака после запятой */
    while (gpsisdigit(*++p)){
      tenk_minutes += mult * (*p - '0');
      mult /= 10;
    }
  }
  return (left / 100) * 100000 + tenk_minutes / 6;
}

static uint32_t gpsatol(const uint8_t *str){
  uint32_t ret = 0;
  while (gpsisdigit(*str))
    ret = 10 * ret + *str++ - '0';
  return ret;
}

static bool_t gpsisdigit(char c){
  return c >= '0' && c <= '9';
}

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */
void GPSInit(void){
  sdStart(&SDGPS, &gps_ser_cfg);

  /* зачистка входной очереди после всех манипуляций. На всякий случай */
  chSysLock();
  chIQResetI( &(SDGPS.iqueue));
  chSysUnlock();

  gps_tp = chThdCreateStatic(
      gpsRxThreadWA,
      sizeof(gpsRxThreadWA),
      GPS_THREAD_PRIO,
      gpsRxThread,
      &SDGPS);

  /* clear time structure */
  gps_timp.tm_isdst = -1;
  gps_timp.tm_wday  = 0;
  gps_timp.tm_mday  = 0;
  gps_timp.tm_yday  = 0;
  gps_timp.tm_mon   = 0;
  gps_timp.tm_year  = 0;
  gps_timp.tm_sec   = 0;
  gps_timp.tm_min   = 0;
  gps_timp.tm_hour  = 0;
}

