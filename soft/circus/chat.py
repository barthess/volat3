#!/usr/bin/python
# -*- coding: utf-8 -*-

""" Assisted GPS sender """

import sys
import os
import time
import socket
import threading

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlinkv10 as mavlink
import mavutil

device = "udp::14550" #сетевой сокет для связи
master = mavutil.mavlink_connection(device)
master.port.settimeout(1)

mav = mavlink.MAVLink(master)
mav.srcSystem = 255 # прикинемся контрольным центром

def heartbeater(mavdevice):
    time.sleep(1)
    mavdevice.heartbeat_send(0, 0, 0, 0, 0)

m = None

print "Awaiting connection..."

while m is None:
    try: m = master.recv_msg()
    except socket.timeout: pass

print "Got it!"
while True:
    heartbeat_th = threading.Thread(target=heartbeater, args=(mav))
    data = sys.stdin.readline()
    mav.statustext_send(0, data)
