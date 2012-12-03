#!/usr/bin/python
# -*- coding: utf-8 -*-

""" Assisted GPS sender """

import sys
import os
import time
import socket

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlinkv10 as mavlink
import mavutil

device = "udp::14550" #сетевой сокет для связи
master = mavutil.mavlink_connection(device)
master.port.settimeout(1)

mav = mavlink.MAVLink(master)
mav.srcSystem = 255 # прикинемся контрольным центром

m = None

print "Awaiting connection..."

while m is None:
    try: m = master.recv_msg()
    except socket.timeout: pass

print "Got it!"
while True:
    mav.mpiovd_agps_send(20, 0, "")
    print "Assistant sent"
    time.sleep(2)
