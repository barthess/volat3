#!/usr/bin/python
# -*- coding: utf-8 -*-


import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial
import time
import socket
import sys
import os
import ConfigParser

from scipy.signal import lfilter

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink

# тип сообщений, который нас интересует
accepted_mav = mavlink.MAVLink_mpiovd_sensors_raw_message

# read socket settings settings from config
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

ADDR = "localhost", config.getint("Socket", "PORT_UDP_TUNER")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ADDR))


# fifo object for mavlink
class fifo(object):
    def __init__(self):
        self.buf = []
    def write(self, data):
        self.buf += data
        return len(data)
    def read(self):
        return self.buf.pop(0)

f = fifo() # buffer for mavlink
mav = mavlink.MAVLink(f)


STEP = 1 # шаг по оси Х
SCOPE_LEN = 100 # количество точек на экране

# import fir
# taps = fir.get_taps()
# lfilter(taps, 1.0, i[1])
# print taps

mag = []

def gen():
    i = 0
    X = []
    m = None
    global mag

    while True:
        try: m = mav.parse_char(sock.recv(4096))
        except mavlink.MAVError: pass

        if m != None:
            print "-----------------------------"
            print m
            if type(m) == accepted_mav:
                mag.append(m.speed)
                if i >= SCOPE_LEN:
                    mag.pop(0)
                else:
                    X.append(i)
                i += STEP
                m = None

        # print mag

