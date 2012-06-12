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

import threading
from multiprocessing import Queue, Lock, Event
from Queue import Empty, Full

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink
import mavutil

import link

SUCCESS = 0
FAILED = 1
LAST_VALUE = 2
TIMEOUT = 3

# read socket settings settings from config
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

# Очереди сообщений
q_tuner  = Queue(8) # эту очередь будет "слушать" тюнер

# create device ID string
# direction specified related to mpiovd
outdevice = "udp:localhost:" + str(config.getint("SocketOut", "PORT_UDP_TUNER"))
indevice  = "udp:localhost:" + str(config.getint("SocketIn",  "PORT_UDP_SERPROXY"))


class QtMav(QtCore.QObject, threading.Thread): #{{{

    param_value_received = QtCore.pyqtSignal(
            mavlink.MAVLink_param_value_message, name="parmValueReceived")

    def __init__(self, outdevice, indevice):
        super(QtMav, self).__init__()
        threading.Thread.__init__(self)
        self.indevice = indevice
        self.outdevice = outdevice

    def refresh(self):
        # open port and request all parameters
        inmaster = mavutil.mavlink_connection(self.indevice, input=False)
        inmaster.target_system = 20
        inmaster.param_fetch_all()
        inmaster.close()

        # wait until all parameters received
        outmaster = mavutil.mavlink_connection(self.outdevice)
        outmaster.port.settimeout(2)
        t = time.time()
        while True:
            try:
                m = outmaster.recv_msg()
            except socket.timeout:
                outmaster.close()
                return TIMEOUT

            if t + 2 < time.time():
                outmaster.close()
                print "timeout"
                return TIMEOUT

            if m is not None:
                if type(m) == mavlink.MAVLink_param_value_message:
                    t = time.time()
                    self.param_value_received.emit(m)
                    print m.param_count, "/", m.param_index
                    if m.param_count == m.param_index + 1:
                        outmaster.close()
                        return LAST_VALUE
#}}}

qt_mav = QtMav(outdevice, indevice)




# create gui
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



# def refresh():#{{{
#     print "refresh clicked"
#     tuner.labelStatusBar.setText("loading values")
#     params.append(g0)
#     params.append(g1)
#     global p, t
#     p = Parameter(name='params', type='group', children=params)
#     t.setParameters(p, showTop=False)
# #}}}
# tuner.buttonRefersh.clicked.connect(refresh)


tuner.buttonRefersh.clicked.connect(qt_mav.refresh)
# tuner.buttonRefersh.pressed.connect(exit())

tuner.show()

sys.exit(app.exec_())
