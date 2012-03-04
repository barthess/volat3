#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import datetime
import ConfigParser
import sys
import os

from struct import *
from multiprocessing import Queue
from multiprocessing import Event
from Queue import Empty, Full # для отлова исключений
import serial
import io
from binascii import hexlify
from localconfig import *

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
    # сделаем файл для записи ошибок
    errlog = open('logs/link.log', 'a')

    #настройка верёвки для связи
    baudrate = config.getint('Link', 'baudrate')
    port     = config.getint('Link', 'port')
    ser = serial.Serial(port, baudrate, timeout = 0.01)
    sio = io.BufferedRWPair(ser, ser, 1024)

    # ждем, пока нас снимут с паузы
    print "---- link input thread ready"
    e_pause.wait()
    print "---- link input thread run"

    while True:
        if e_kill.is_set():
            print "=== Link input thread. Kill signal received. Exiting"
            return
        c = sio.read()
        try:
            m = mav.parse_char(c)
        except mavlink.MAVError:
            pass

        if m != None:
            if type(m) == mavlink.MAVLink_uvvu_sensors_raw_short_message:
                try:
                    q_tlm.put_nowait(m)
                except Full:
                    errlog.write(str(datetime.datetime.now()) + " -- Telemetry queue full\n")
                m = None
            elif type(m) == mavlink.MAVLink_heartbeat_message:
                print "Heartbeat"
                m = None
            else:
                print "I do not know what to do with this message."
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

