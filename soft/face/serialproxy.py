#!/usr/bin/python
# -*- coding: utf-8 -*-

import ConfigParser
import socket
import time
import datetime
import serial
from binascii import hexlify

from utils import *
import globalflags
flags = globalflags.flags

#
errlog = open('logs/proxy.log', 'a')

# load settings from config file
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

# port list from config
portlist = []
for i in config.items("Socket"):
    portlist.append(config.getint("Socket", i[0]))

# SOCK_DGRAM is the socket type to use for UDP sockets
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# serial port
bufsize  = config.getint('Serial', 'bufsize')
baudrate = config.getint('Serial', 'baudrate')
port     = config.get('Serial', 'port')
ser      = serial.Serial(port, baudrate, timeout = 0.5)

#TODO: write all bytes to file
# infinitely read serial port and push data to sockets
while True:
    c = ser.read(bufsize)
    if len(c) != 0:
        for i in portlist:
            sock.sendto(c, ("localhost", i))
    else:
        errlog.write(str(datetime.datetime.now()) + " -- connection interrupted\n")


