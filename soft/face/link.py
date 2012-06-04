#!/usr/bin/python
# -*- coding: utf-8 -*-

import time
import datetime
import ConfigParser
import sys
import os
import socket
from struct import *
import serial
from binascii import hexlify

from multiprocessing import Queue
from multiprocessing import Event
from Queue import Empty, Full # для отлова исключений

from utils import *
import globalflags
flags = globalflags.flags

# read settings from file
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

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

# Socket settings
ADDR = "localhost", config.getint("Socket", "PORT_UDP_HUD")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ADDR))

# файл для записи ошибок
errlog = open('logs/link.log', 'a')


def __receive_data(q_tlm, q_log):
    """ Парсит байты из сетевого сокета в сообщения """#{{{
    m = None
    c = sock.recv(1024)

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
#}}}
def linkin(q_tlm, q_log, e_pause, e_kill, config):
    """ Менеджер входящих сообщений. """ #{{{
    # ждем, пока нас снимут с паузы
    dbgprint("**** link input thread ready")
    e_pause.wait()
    dbgprint("**** link input thread run")

    while True:
        if e_kill.is_set():
            dbgprint("**** Link input thread. Kill signal received. Exiting")
            return
        else:
            __receive_data(q_tlm, q_log)
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

