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

class HeartbeatSender(threading.Thread):
    """ Class sending heartbeats """

    def __init__(self, mavdevice):
        threading.Thread.__init__(self)
        self.mavdevice = mavdevice
        self.__stop = threading.Event()

    def stop(self):
        self.__stop.set()

    def run(self):
        print "*** heartbeter started"
        while not (self.__stop.is_set()):
            time.sleep(1)
            self.mavdevice.heartbeat_send(mavlink.MAV_TYPE_GCS, mavlink.MAV_AUTOPILOT_INVALID, 192, 0, mavlink.MAV_STATE_ACTIVE)
        print "*** heartbeater stopped"

##################################################
# main cycle
m = None

print "Awaiting connection..."

while m is None:
    try: m = master.recv_msg()
    except socket.timeout: pass

print "Got it!"
heartbeat_th = HeartbeatSender(mav)
heartbeat_th.start()

while True:
    try:
        s = sys.stdin.readline()
        if len(s) > 50:
            s = s[0:49]
            print "WARNING: message was truncated to first 50 symbols"
    except KeyboardInterrupt:
        print "*** stopping heartbeat thread"
        heartbeat_th.stop()
        heartbeat_th.join()
        print "*** exiting. By."
        exit()

    strutf8 = unicode(s, "utf-8")
    try:
        mav.statustext_send(0, str(strutf8))
    except UnicodeEncodeError:
        print "ERROR: Cyrillic doest not supported yet."


