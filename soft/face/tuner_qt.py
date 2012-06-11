#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
import sys
import os
import ConfigParser
import time
import collections
import user
import socket

from pyqtgraph.Qt import QtCore, QtGui
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree, ParameterItem, registerParameterType
from PyQt4 import uic

from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full
import link


# Очереди сообщений
q_tuner  = Queue(4) # эту очередь будет "слушать" тюнер

# настройка и запуск процесса связи {{{
# события для блокирования до тех пор, пока событие в состоянии clear
e_pause = Event() # лок для постановки процесса на паузу
e_pause.clear()
e_kill = Event() # предложение умереть добровольно
e_kill.clear()

# create link thread
# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink

# read socket settings settings from config
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

ADDR = "localhost", config.getint("SocketOut", "PORT_UDP_TUNER")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ADDR))

p_linkin = Process(target=link.input, args=(q_tuner, e_pause, e_kill, sock, ))
p_linkin.start()
e_pause.set()
#}}}









app = QtGui.QApplication(sys.argv)

f = open("tuner_ui.py.tmp", "w")
uic.compileUi("tuner.ui", f)
tuner = uic.loadUi("tuner.ui")



p0 = {'name': 'Param 1', 'type': 'int', 'value': 10}
p1 = {'name': 'Param 2', 'type': 'float', 'value': 10}

g0 = {'name': 'Group 0', 'type': 'group', 'children': [p0, p1]}
g1 = {'name': 'Group 1', 'type': 'group', 'children': [p0, p1]}

params = []
p = Parameter(name='params', type='group', children=params)
t = ParameterTree(parent=tuner.tabRawTree)
t.setParameters(p, showTop=False)
# print tuner.tabRawTree.width()
# t.resize(400, 500)


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def refresh():#{{{
    sock.sendto("test", ("localhost", 14555))
    print "refresh clicked"
    tuner.labelStatusBar.setText("loading values")
    params.append(g0)
    params.append(g1)
    global p, t
    p = Parameter(name='params', type='group', children=params)
    t.setParameters(p, showTop=False)
#}}}


tuner.buttonRefersh.clicked.connect(refresh)

tuner.show()
e_kill.set()
p_linkin.join()

sys.exit(app.exec_())
