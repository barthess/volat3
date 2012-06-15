#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import socket
from binascii import hexlify

from multiprocessing import Queue
from multiprocessing import Event
from Queue import Empty, Full # для отлова исключений

from utils import *
import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavutil

def input(q, e_pause, e_kill, device):#{{{
    """ Менеджер входящих сообщений.
    q -- очередь сообщений, в которую надо складывать успешно принятые пакеты
    device -- сетевой сокет, из которого сыпятся байты, теоретически содержащие пакеты """

    # ждем, пока нас снимут с паузы
    dbgprint("**** link input thread ready")
    e_pause.wait()
    dbgprint("**** link input thread run")

    master = mavutil.mavlink_connection(device)
    master.port.settimeout(1)

    m = None

    while True:
        if e_kill.is_set():
            dbgprint("**** Link input thread. Sigterm received. Exiting")
            return

        try: m = master.recv_msg()
        except socket.timeout: pass

        if m is not None:
            try: q.put_nowait(m)
            except Full: pass
#}}}

