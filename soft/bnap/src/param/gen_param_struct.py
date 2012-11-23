#!/usr/bin/env python
# -*- coding: utf-8 -*-

# C name of array with values in RAM
arr = "gp_val"

SEND_MIN    = 1
SEND_MAX    = (24 * 60 * 60 * 1000)

ONBOARD_PARAM_NAME_LENGTH = 15

#    key            min         default max         type checker    help
param = [
("SYS_ID",          1,          20,     255,        "u", "NULL",    "System ID.\\n This value MUST BE FIRST in param structure. Value 0 reserved for ground station."),
("SYS_mavtype",     0,          10,     16,         "u", "NULL",    "Autopilot type (0 - generic, 1 - fixed wing, 10 - ground rover).\\nOther types you can found in enum MAV_TYPE \\nNOTE! You MUST REBOOT device after changing it."),
("SH_enable",       0,          0,      1,          "u", "NULL",    "Set to 1 to switch off telemetry sending and enabling shell."),

#/* intervals between sending different data (mS) */
("T_heartbeat",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_mpiovd_data",   SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_sys_status",    SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "Interval of sending this data in milliseconds.\\nSet it to 0 for disabling"),
("T_sys_time",      SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "Timeout help here"),
("T_text",          SEND_MIN,   1,      SEND_MAX,   "u", "sendtmo", "NULL"),
("T_gps_int",       SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_gps_raw",       SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_param_value",   SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved1",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved2",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved3",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved5",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved6",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),
("T_reserved7",     SEND_MIN,   1000,   SEND_MAX,   "u", "sendtmo", "NULL"),

#/**** Variables for debugging purposes ****/
("DBG_reserved0",   0,          0,      10000,      "u", "NULL",    "NULL"),
("DBG_reserved1",   0,          0,      10000,      "u", "NULL",    "NULL"),
("DBG_reserved2",   0,          0,      10000,      "u", "NULL",    "NULL"),

#/**** fake field with 14 symbols name ****/
("param_end_mark",  0,          0,      1224,       "u", "NULL",    "Fake parameter with maximum allowable name length"),
]

######### checkers ###############
# name lengths
for i in param:
    if len(i[0]) > ONBOARD_PARAM_NAME_LENGTH - 1: # -1 because last char must be 0x00
        print "ERROR: name", '"'+i[0]+'"', "is more than", ONBOARD_PARAM_NAME_LENGTH - 1, "characters"
        exit()

# name duplications
n = 0
while n < len(param):
    p = n + 1
    while p < len(param):
        if param[n][0] == param[p][0]:
            print "ERROR: duplicated parameter names detected"
            print param[n][0]
            exit()
        p += 1
    n += 1

# values' types
for i in param:
    if i[4] == "u" or i[4] == "i":
        if (type(i[1]) is not int) or (type(i[2]) is not int) or (type(i[3]) is not int):
            print "ERROR: type set to integer but given vales are floats."
            print i
            exit()
    if i[4] == "u" and ((i[1] < 0) or (i[2] < 0) or (i[1] < 0)):
        print "ERROR: type set to unsigned but given values are negative."
        print i
        exit()

    if (i[4] != "u") and (i[4] != "i") and (i[4] != "f"):
        print "ERROR: unsupported type"
        print i
        exit()

# boundaries
for i in param:
    if i[5] == "NULL": # for default checker
        if (i[1] <= i[2]) and (i[2] <= i[3]) and (i[1] != i[3]):
            pass
        else:
            print "ERROR: bad boundaries in", i[0]
            exit()
    else: # for different checkers
        if i[1] >= i[3]:
            print "ERROR: bad boundaries in", i[0]
            exit()


# print header
print "// This file is automatically generated."
print "// Do not edit it"
print "// open gen_param_struct.py, edit struct definition and run it to generate new file"
print "const int32_t OnboardParamCount =", str(len(param)) +";"
print """
/*
 * variable array of parameters in RAM
 */ """
print "static floatint", arr+"["+str(len(param))+"];"
print """
/**
 *
 */ """
print "const GlobalParam_t GlobalParam[] = {"


########### generator #############
n = 0
for i in param:
    # delete pythonic new line symbols and enclose in quotes text strings
    if i[6] != "NULL":
        helpstr = ""
        for s in i[6]:
            if s != "\n":
                helpstr += s
        helpstr = "\"" + helpstr + "\""
    else:
        helpstr = "NULL"

    # create checker function string
    if i[5] != "NULL":
        checker = "&_" + i[5] + "_param_checker"
    else:
        checker = "NULL"

    # create format string
    if i[4] == "u":
        fmt = "{\"%s\", {.u32 = %u}, {.u32 = %u}, {.u32 = %u}, &%s[%u], %s, MAVLINK_TYPE_UINT32_T, %s},"
    elif i[4] == "i":
        fmt = "{\"%s\", {.i32 = %d}, {.i32 = %d}, {.i32 = %d}, &%s[%u], %s, MAVLINK_TYPE_INT32_T,  %s},"
    elif i[4] == "f":
        fmt = "{\"%s\", {.f32 = %F}, {.f32 = %F}, {.f32 = %F}, &%s[%u], %s, MAVLINK_TYPE_FLOAT,    %s},"
    else:
        print "ERROR: unsupported type"
        exit()

    print fmt % (i[0],         i[1],         i[2],        i[3], arr, n,  checker,                  helpstr)
    n += 1

print "};"
