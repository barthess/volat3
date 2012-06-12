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

def __receive_data(q, c):#{{{
    """ Parses input bytes and push _all_ decoded messages in queue """
    m = None

    try:
        m = mav.parse_char(c)
    except mavlink.MAVError:
        errlog.write(str(datetime.datetime.now()) + " -- Mavlink error\n")
        pass

    if m != None:
        try:
            q.put_nowait(m)
        except Full:
            errlog.write(str(datetime.datetime.now()) + " -- Telemetry queue is full\n")
        dbgprint(m)
        m = None
#}}}
def input(q, e_pause, e_kill, sock):#{{{
    """ Менеджер входящих сообщений.
    q -- очередь сообщений, в которую надо складывать успешно принятые пакеты
    sock -- сетевой сокет, из которого сыпятся байты, теоретически содержащие пакеты """
    # ждем, пока нас снимут с паузы
    dbgprint("**** link input thread ready")
    e_pause.wait()
    dbgprint("**** link input thread run")

    while True:
        if e_kill.is_set():
            dbgprint("**** Link input thread. Sigterm received. Exiting")
            return
        c = sock.recv(1024)
        __receive_data(q, c)
#}}}

