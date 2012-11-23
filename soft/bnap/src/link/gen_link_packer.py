#!/usr/bin/env python

#       message name            mail struct for it      mailbox for it
names = {
        "Dm" : ["heartbeat", "gps_raw_int", "global_position_int", "system_time", "mpiovd_sensors", "sys_status", "statustext"],
        "Cc" : ["heartbeat", "gps_raw_int", "global_position_int", "system_time", "mpiovd_sensors", "sys_status", "statustext"]
        }

def head(f):
    f.write("""
#include "ch.h"
#include "hal.h"
#include "mavlink.h"
#include "utils.h"
#include "mpiovd.h"
#include "link.h"
#include "message.h"
#include "main.h"

extern mavlink_system_t mavlink_system_struct;

""")

def foot(f, name):
    f.write("""
    default:
      goto WAIT;
      break;
    }
    /* write in port after exiting from case */
    len = mavlink_msg_to_send_buffer(sendbuf, &mavlink_message_struct);
    """)
    f.write(str.lower(name) + "_sdWrite(sdp, sendbuf, len);")
    f.write("""
  }
}""")

def gen(name, arr):
    f = open("./src/link/link_" + str.lower(name) + "_packer.c", 'w')
    f.write("/* \n!!! Atomatically generated by\n" + __file__ + "\nDo not edit it manually. \n*/")
    head(f)
    f.write("#include \"link_" + str.lower(name) + "_packer.h\"\n\n")

    for i in arr:
        f.write("extern mavlink_" + i + "_t mavlink_" + i + "_struct;\n")
        f.write("extern EventSource event_mavlink_" + i + ";\n\n")

    for i in arr:
        f.write("static systime_t " + i + "_lastsent = 0;\n")

    f.write("\nvoid " + name + "PackCycle(SerialDriver *sdp){\n")
    for i in arr:
        f.write("  struct EventListener el_" + i + ";\n")
        f.write("  chEvtRegisterMask(&event_mavlink_" + i + ", &el_" + i + ", EVMSK_MAVLINK_" + str.upper(i) + ");\n\n")
    f.write("  eventmask_t evt = 0;\n")
    f.write("  mavlink_message_t mavlink_message_struct;\n")
    f.write("  uint8_t sendbuf[MAVLINK_MAX_PACKET_LEN];\n")
    f.write("  uint16_t len = 0;\n")
    f.write("  while (!chThdShouldTerminate()) {\n")
    f.write("WAIT:\n")
    st = ""
    for i in arr:
        st += "EVMSK_MAVLINK_" + str.upper(i) + " | "
    st = st[0:-3]
    f.write("    evt = chEvtWaitOne(" + st + ");\n")
    f.write("    switch(evt){\n")

    for i in arr:
        f.write("    case EVMSK_MAVLINK_" + str.upper(i) + ":\n")
        f.write("      if(FALSE == traffic_limiter(&" + i + "_lastsent, " + i + "_sendperiod))\n")
        f.write("        goto WAIT;\n")
        f.write("      memcpy_ts(sendbuf, &mavlink_" + i + "_struct, sizeof(mavlink_" + i +"_struct), 4);\n")
        f.write("      mavlink_msg_" + i + "_encode(mavlink_system_struct.sysid, MAV_COMP_ID_ALL, &mavlink_message_struct, (mavlink_" + i + "_t *)sendbuf);\n")
        f.write("      break;\n\n")
    foot(f, name)
    f.close()


for key in names.keys():
    gen(key, names[key])
