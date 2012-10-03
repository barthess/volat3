#ifndef PARAM_H_
#define PARAM_H_


/* периодичность посылки данных в милисекундах */
#define SEND_MIN                  20
#define SEND_MAX                  5000
#define SEND_OFF                  SEND_MIN - 1


#define ONBOARD_PARAM_NAME_LENGTH 15
#define PARAM_ID_SIZE             16


typedef struct GlobalParam_t GlobalParam_t;

/**
 * Combined data type for use in mavlink
 */
typedef union{float f32; int32_t i32; uint32_t u32;} floatint;

/**
 * Global parameter
 */
struct GlobalParam_t
{
  const char *name;
  const floatint min;
  floatint value;
  const floatint max;
  const uint8_t param_type;
};


//typedef struct GlobalParam_t GlobalParam_t;
//struct GlobalParam_t
//{
//  const char *name;
//  union{
//    float    f32;
//    uint32_t u32;
//    int32_t  i32;
//    uint16_t u16;
//    int16_t  i16;
//    uint8_t  u8;
//    int8_t   i8;
//  }value;
//  const float min;
//  const float max;
//  const uint8_t param_type;
//};

/**
 *
 */
typedef enum {
  PARAM_OK = 1,
  PARAM_NOT_CHANGED = 2,  /* parameter allready contain this value */
  PARAM_CLAMPED = 3,      /* value claped to limits */
  PARAM_INCONSISTENT = 4, /* NaN or INF */
  PARAM_WRONG_TYPE = 5,   /* unsuppoerted parameter type */
} param_status_t;

bool_t set_global_param(void *value,  GlobalParam_t *param);
void ParametersInit(void);
int32_t _key_index_search(const char* key);
void *ValueSearch(char *str);

#endif /* PARAM_H_ */
