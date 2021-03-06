#include <stdio.h>

#include "ch.h"
#include "hal.h"

#include "cli.h"
#include "param_cli.h"
#include "param.h"
#include "persistant.h"
#include "main.h"

#include "../microrl/src/config.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */
extern GlobalParam_t global_data[];
extern uint32_t OnboardParamCount;

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
/*
 * confirmation of changes
 */
static void _param_cli_confirm(param_status_t status){
  if (status == PARAM_OK)
    return;
    //cli_println("Success");
  else if (status == PARAM_CLAMPED)
    cli_println("WARNING: value clamped to safety limits.");
  else if (status == PARAM_NOT_CHANGED)
    cli_println("WARNING: value not changed.");
  else if (status == PARAM_INCONSISTENT)
    cli_println("ERROR: value inconsistent.");
  else
    cli_println("ERROR: Unhandled error.");
}

/**
 *
 */
static void _param_cli_print(uint32_t i, bool_t need_help){

  const int n = 70;
  int nres = 0;
  char str[n];

  nres = snprintf(str, n, "%-15s", global_data[i].name);
  cli_print_long(str, n, nres);

  switch(global_data[i].param_type){
  case MAVLINK_TYPE_FLOAT:
    /* Float values unsupported. Need to realize manual printing */
//    nres = snprintf(str, n, " %-15f %-15f %-15f",
//        global_data[i].min.f32,
//        global_data[i].value.f32,
//        global_data[i].max.f32);
    nres = snprintf(str, n, " %s", "Float values currently unsupported");
    break;
  case MAVLINK_TYPE_INT32_T:
    nres = snprintf(str, n, " %-15d %-15d %-15d",
        (int)global_data[i].min.i32,
        (int)global_data[i].value.i32,
        (int)global_data[i].max.i32);
    break;
  default: // uint32_t
    nres = snprintf(str, n, " %-15u %-15u %-15u",
        (unsigned int)global_data[i].min.u32,
        (unsigned int)global_data[i].value.u32,
        (unsigned int)global_data[i].max.u32);
    break;
  }

  cli_print_long(str, n, nres);
  cli_print(ENDL);

  if (need_help){
    cli_println("");
  }
}

/**
 *
 */
void _param_cli_print_header(void){
  cli_println("Name            min             value           max");
  cli_println("--------------------------------------------------------------");
}

/**
 *
 */
static void _param_print_all(void){
  uint32_t i = 0;

  _param_cli_print_header();

  for (i = 0; i < OnboardParamCount; i++)
    _param_cli_print(i, FALSE);
}

/**
 *
 */
static param_status_t _param_cli_set(const char * val, uint32_t i){
  floatint v;
  bool_t set_status = PARAM_FAILED;
  int sscanf_status;

  switch(global_data[i].param_type){
  case MAVLINK_TYPE_FLOAT:
    sscanf_status = sscanf(val, "%f", &v.f32);
    break;

  case MAVLINK_TYPE_INT32_T:
    sscanf_status = sscanf(val, "%i", (int*)&v.i32);
    break;

  default: // uint32_t
    sscanf_status = sscanf(val, "%u", (unsigned int*)&v.u32);
    break;
  }

  if (sscanf_status != 1)
    return PARAM_INCONSISTENT;
  else
    set_status = set_global_param(&v, &global_data[i]);

  if (set_status == PARAM_SUCCESS)
    return PARAM_OK;
  else
    return PARAM_CLAMPED;
}

/**
 *
 */
static void _param_cli_help(void){
  cli_println("Run without parameters to get full parameters list.");
  cli_println("'param save' to save parameters to EEPROM.");
  cli_println("'param help' to get this message.");
  cli_println("'param PARAM_name' to get value of parameter.");
  cli_println("'param PARAM_name N' to set value of parameter to N.");
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 * Working with parameters from CLI.
 */
Thread* param_clicmd(int argc, const char * const * argv, SerialDriver *sdp){
  (void)sdp;

  int32_t i = -1;
  param_status_t status;

  /* wait until value uninitialized (just to be safe) */
  while (OnboardParamCount == 0)
    chThdSleepMilliseconds(10);

  /* no arguments */
  if (argc == 0)
    _param_print_all();

  /* one argument */
  else if (argc == 1){
    if (strcmp(*argv, "help") == 0)
      _param_cli_help();
    else if (strcmp(*argv, "save") == 0){
      cli_print("Please wait. Saving to EEPROM... ");
      save_params_to_eeprom();
      cli_println("Done.");
    }
    else{
      i = _key_index_search(*argv);
      if (i != -1){
        _param_cli_print_header();
        _param_cli_print(i, TRUE);
      }
      else{
        cli_println("ERROR: unknown parameter name.");
      }
    }
  }

  /* two arguments */
  else if (argc == 2){
    i = -1;
    i = _key_index_search(argv[0]);
    if (i != -1){
      status = _param_cli_set(argv[1], i);
      _param_cli_confirm(status);
    }
    else{
      cli_println("ERROR: unknown parameter name.");
    }
  }
  else{
    cli_println("ERROR: bad arguments. Try 'param help'.");
  }

  /* stub */
  return NULL;
}

