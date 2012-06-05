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

import fir
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
SCOPE_LEN = 1000 # количество точек на экране

fig = plt.figure()
ax = fig.add_subplot(111)
# origin, = ax.plot([], [], ',', lw=1, antialiased=False)
origin, = ax.plot([], [], lw=1)
filtered, = ax.plot([], [], lw=4, color='red')
ax.set_xlim(0, SCOPE_LEN)
ax.set_ylim(-5, 50)

taps = fir.get_taps()
print taps

def animate(i):#{{{ функция анимирует
    origin.set_data(i)
    filtered.set_data(i[0], lfilter(taps, 1.0, i[1]))
    xmin, xmax = ax.get_xlim()
    # ax.set_xlim( (xmin + STEP, xmax + STEP) )
    return origin, filtered
#}}}



def gen():
    i = 0
    X = []
    mag = []
    m = None

    while True:
        try: m = mav.parse_char(sock.recv(1024))
        except mavlink.MAVError: pass

        if m != None:
            if type(m) == accepted_mav:
                mag.append(m.speed)
                if i >= SCOPE_LEN:
                    mag.pop(0)
                else:
                    X.append(i)
                i += STEP
                m = None

        print mag
        yield X, mag





# Init only required for blitting to give a clean slate.
def init():
    origin.set_data(0, 0)
    filtered.set_data(0, 0)
    # xmin, xmax = ax.get_xlim()
    # ax.set_xlim( (xmin + STEP, xmax + STEP) )
    return origin, filtered
    # origin.set_ydata(np.ma.array(L, mask=True))

# ani = animation.FuncAnimation(fig, animate, gen, interval=25, blit=False)
ani = animation.FuncAnimation(fig, animate, gen, interval=5, init_func=init, blit=True)
# ani = animation.FuncAnimation(fig, animate, gen, interval=25, blit=True)
plt.show()




