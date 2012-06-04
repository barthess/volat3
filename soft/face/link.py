#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import datetime
import sys
import os
from binascii import hexlify

from multiprocessing import Queue
from multiprocessing import Event
from Queue import Empty, Full # для отлова исключений

from utils import *
import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink

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

# файл для записи ошибок
errlog = open('logs/link.log', 'a')

def __receive_data(q_tlm, c):#{{{
    """ Parses input bytes and push _all_ decoded messages in queue """
    m = None

    try:
        m = mav.parse_char(c)
    except mavlink.MAVError:
        errlog.write(str(datetime.datetime.now()) + " -- Mavlink error\n")
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
            m = None
#}}}

def linkin(q, e_pause, e_kill, sock):#{{{
    """ Менеджер входящих сообщений. """
    # ждем, пока нас снимут с паузы
    dbgprint("**** link input thread ready")
    e_pause.wait()
    dbgprint("**** link input thread run")

    while True:
        if e_kill.is_set():
            dbgprint("**** Link input thread. Sigterm received. Exiting")
            return
        else:
            c = sock.recv(1024)
            __receive_data(q, c)
#}}}

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

