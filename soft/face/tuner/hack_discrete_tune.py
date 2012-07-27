#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
import sys
import os
import ConfigParser
import time
import collections
import user
import socket
import math

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../../mavlink/python'))
import mavlink
import mavutil

# read socket settings settings from config
config = ConfigParser.SafeConfigParser()
config.read('../default.cfg')

# create device ID string
# direction specified related to mpiovd
outdevice = "udp:localhost:" + str(config.getint("SocketOut", "PORT_UDP_TUNER"))
indevice  = "udp:localhost:" + str(config.getint("SocketIn",  "PORT_UDP_SERPROXY"))


def write_rom():
    inmaster = mavutil.mavlink_connection(indevice, input=False)
    inmaster.target_system = 20

    v = 32767

    param_id = "REL_Z_0"
    inmaster.mav.param_set_send(20, 0, param_id, v, 5)
    time.sleep(0.1)

    # param_id = "REL_Z_32"
    # inmaster.mav.param_set_send(20, 0, param_id, v, 5)
    # time.sleep(0.1)

    # param_id = "REL_VCC_0"
    # inmaster.mav.param_set_send(20, 0, param_id, v, 5)
    # time.sleep(0.1)

    # param_id = "REL_VCC_32"
    # inmaster.mav.param_set_send(20, 0, param_id, v, 5)
    # time.sleep(0.1)

write_rom()


