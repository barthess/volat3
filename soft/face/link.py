#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import datetime
import ConfigParser
import sys
import os
import socket

from struct import *
from multiprocessing import Queue
from multiprocessing import Event
from Queue import Empty, Full # для отлова исключений
import serial
import io
from binascii import hexlify

from utils import *

import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink


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



def linkin(q_tlm, q_log, e_pause, e_kill, config):
    """
    менеджер сообщений
    Принимает поток данных, выискивает пакеты, сортирует.

    """
    # UDP сокет для всех желающих
    # SOCK_DGRAM is the socket type to use for UDP sockets
    port_udp_agc = config.getint('Link', 'PORT_UDP_QGC')
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # файл для записи ошибок
    errlog = open('logs/link.log', 'a')

    #настройка верёвки для связи по ком-порту
    baudrate = config.getint('Link', 'baudrate')
    port     = config.get('Link', 'port')
    ser = serial.Serial(port, baudrate, timeout = 1)
    # TODO: create mpiovd presence detector based on empty string returned on
    # timeout
    sio = io.BufferedRWPair(ser, ser, 1024)
    m = None

    # ждем, пока нас снимут с паузы
    dbgprint("**** link input thread ready")
    e_pause.wait()
    dbgprint("**** link input thread run")

    while True:
        if e_kill.is_set():
            dbgprint("**** Link input thread. Kill signal received. Exiting")
            return
        # c = sio.read()
        c = ser.read()
        # send byte via udp
        sock.sendto(c, ("localhost", port_udp_agc))

        try:
            m = mav.parse_char(c)
            # print hexlify(c)
        except mavlink.MAVError:
            pass

        if m != None:
            # print hexlify(m)
            # if type(m) == mavlink.MAVLink_mpiovd_sensors_scaled_message:
            #     try:
            #         q_tlm.put_nowait(m)
            #     except Full:
            #         errlog.write(str(datetime.datetime.now()) + " -- Telemetry queue is full\n")
            #     dbgprint(m)
            #     m = None

            if type(m) == mavlink.MAVLink_mpiovd_sensors_raw_message:
                try:
                    q_tlm.put_nowait(m)
                except Full:
                    errlog.write(str(datetime.datetime.now()) + " -- Telemetry queue is full\n")
                dbgprint(m)
                m = None

            elif type(m) == mavlink.MAVLink_heartbeat_message:
                dbgprint(m)
                m = None

            else:
                # dbgprint("I do not how to handle message of type " + str(type(m)) + " .")
                m = None



#def linkout(q_out, e_pause, e_kill):
#    """
#    менеджер сообщений
#    Принимает поток данных, выискивает пакеты, сортирует
#
#    """
#
#    xbee_response = None # пришедший с модема пакет
#    msg_up = None # строка, которую надо запихнуть в пакет для отправки
#
#    # ждем, пока нас снимут с паузы
#    print "---- linkout ready"
#    e_pause.wait()
#    print "---- linkout run"
#
#    while True:
#        if e_kill.is_set():
#            print "=== Linkin. Kill signal received. Exiting"
#            return
#
#        try: msg_out = q_out.get_nowait()
#        except Empty: pass
#
#        if msg_out != None: # пихаем в провод
#            c = sio.write()
#            msg_out = None
#

