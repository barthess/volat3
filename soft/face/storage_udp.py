#!/usr/bin/python
# -*- coding: utf-8 -*-

""" Assisted GPS sender """

import sys
import os
import time
import socket

from utils import *
import globalflags
flags = globalflags.flags

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

def getcount():
    m = None
    retry = 50
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


print "Awaiting connection..."
while m is None:
    try: m = master.recv_msg()
    except socket.timeout: pass
print "Got it!"
while True:
    data = sys.stdin.readline()
    if data == "?\n":
        mav.oblique_storage_request_count_send(20, 0)
        getcount()
    else:
        print "unrecognized command"
