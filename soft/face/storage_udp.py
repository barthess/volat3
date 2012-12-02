#!/usr/bin/python
# -*- coding: utf-8 -*-

""" Assisted GPS sender """

import sys
import os
import time
import socket
import argparse

from utils import *
import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlinkv10 as mavlink
import mavutil

# command line parser
parser = argparse.ArgumentParser()
parser.add_argument('-s','--start',
        type=int,
        # default=0,
        help='first data block to read (default: %(default)s)')
parser.add_argument('-f','--finish',
        type=int,
        # default=255,
        help='last data block to read (default: %(default)s)')
parser.add_argument('-d','--device',
        type=str,
        default="udp::14550",
        help='device for data exchange (default: %(default)s)')
args = parser.parse_args()

device = args.device # устройство для связи
master = mavutil.mavlink_connection(device, append=True)
try: master.port.settimeout(2)
except AttributeError: pass
mav = mavlink.MAVLink(master)
mav.srcSystem = 255 # прикинемся контрольным центром

m = None

def getcount():#{{{
    m = None
    retry = 2000

    mav.oblique_storage_request_count_send(20, 0)

    while (type(m) is not mavlink.MAVLink_oblique_storage_count_message) and retry > 0:
        try:
            m = master.recv_msg()
            if type(m) == mavlink.MAVLink_oblique_storage_count_message:
                print m.count
                return
        except socket.timeout:
            pass
        retry -= 1
    print "No answer"
    #}}}

def getdata(s, f):
    mav.oblique_storage_request_send(20, 0, s, f)
    while s < f:
        try:
            m = master.recv_msg()
            # print type(m)
            if type(m) == mavlink.MAVLink_gps_raw_int_message:
                print "gps_raw_int   ", m.time_usec / 1000
                s += 0.5
            elif type(m) == mavlink.MAVLink_mpiovd_sensors_message:
                print "mpiovd_sensors", m.time_usec / 1000
                s += 0.5
        except socket.timeout:
            pass
    print "done"


print "Awaiting connection..."
while (type(m) is not mavlink.MAVLink_heartbeat_message):
    try: m = master.recv_msg()
    except socket.timeout: pass
print "Got it!"

if (args.start is None) or (args.finish is None):
    print "available message count is", getcount()
else:
    getdata(args.start, args.finish)


