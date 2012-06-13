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
                    # print m.param_count, "/", m.param_index, m.param_id, m.param_value
                    # print m.param_id.split('_')[0]
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


params = []
parameter = Parameter(name='params', type='group', children=params)
parameter_tree = ParameterTree(parent=tuner.tabRawTree)
parameter_tree.setParameters(parameter, showTop=False)
parameter_tree.resize(300, 500)





class VolatCheckbox(QtGui.QCheckBox):
    """ Subclass of standard Qt checkbox """

    """ n -- number of channel """
    mytoggled = QtCore.pyqtSignal(bool, int)

    def __init__(self, n, parent):
        """ n -- number of corresponding input channel """
        super(QtGui.QCheckBox, self).__init__(parent=parent)
        self.chnum = n
        self.toggled.connect(self.myemit)

    def myemit(self, state):
        self.mytoggled.emit(state, self.chnum)

def vtest(state, n):
    print state, n

volatch = VolatCheckbox(10, parent=tuner.tabHelp)
volatch.mytoggled.connect(vtest)



class RelCheckboxGrid(QtGui.QWidget):#{{{

    def __init__(self, parent):
        super(QtGui.QWidget, self).__init__(parent=parent)
        self.state = 0 # bitmask of all checkboxes
        self.grid = [] # array of checkbox objects

    def addCheckbox(self, i, st):
        step = 16
        x0 = 5
        y0 = 25
        self.grid.append(QtGui.QCheckBox(parent=self,
                        text=QtCore.QString.fromUtf8(st),
                        geometry=QtCore.QRect(x0, y0 + i * step, 350, 20)))

    def setState(self, state):
        i = 0
        while i < len(self.grid):
            self.grid[i].setChecked((state >> i) & 1)
            i += 1

    def getState(self):
        i = 0
        while i < 32:
            print self.grid[i].checkState()
            i += 1

chRel_0_31 = RelCheckboxGrid(tuner.tabRelay_0_31)
for i in range(0, 31):
    chRel_0_31.addCheckbox(i, config.get("Rel0_31", "ch"+str(i)))

chRel_32_63 = RelCheckboxGrid(tuner.tabRelay_32_63)
for i in range(0, 31):
    chRel_32_63.addCheckbox(i, config.get("Rel32_63", "ch"+str(i+32)))

chRel_0_31.setState(0xFFFF)
chRel_32_63.setState(0xFFFF00)
# grid_0_31.getState()
#}}}


def get_group_index(params, gname):#{{{
    """
    recursive group refreshing/adding
    params -- parameters structure
    gname -- string representing name
    return:
        index in params structure
        -1 if not found
    """
    i = 0
    for g in params:
        if g['name'] == gname:
            return i
        i += 1
    return -1
#}}}
def get_param_index(children, pname):#{{{
    """
    recursive param refreshing/adding
    children -- array of parameters
    pname -- string representing name
    return:
        index in children
        -1 if not found
    """
    i = 0
    for g in children:
        if g['name'] == pname:
            return i
        i += 1
    return -1
#}}}
def get_param_type(m):#{{{
    """
    determine parameter type based on mavlink defines
    """
    MAVLINK_TYPE_CHAR     = 0
    MAVLINK_TYPE_UINT8_T  = 1
    MAVLINK_TYPE_INT8_T   = 2
    MAVLINK_TYPE_UINT16_T = 3
    MAVLINK_TYPE_INT16_T  = 4
    MAVLINK_TYPE_UINT32_T = 5
    MAVLINK_TYPE_INT32_T  = 6
    MAVLINK_TYPE_FLOAT    = 9

    if m.param_type == MAVLINK_TYPE_FLOAT:
        return "float"
    else:
        return "int"
#}}}
def refresh_table(m):#{{{
    s = "got: " + str(m.param_count)+"/"+str(m.param_index + 1) + " " + m.param_id
    tuner.labelStatusBar.setText(s)
    # print m.param_index
    gname = m.param_id.split('_')[0]
    pname = m.param_id.split('\x00')[0]
    ptype = get_param_type(m)

    ptmp = {'name': pname, 'type': ptype, 'value': m.param_value}

    gindex = get_group_index(params, gname)
    if gindex == -1:
        params.append({'name': gname, 'type': 'group', 'children': [ptmp]})
    else:
        pindex = get_param_index(params[gindex]['children'], pname)
        if pindex == -1:
            params[gindex]['children'].append(ptmp)
        else:
            params[gindex]['children'][pindex]['value'] = m.param_value

    if m.param_count == m.param_index + 1:
        global parameter, parameter_tree
        parameter = Parameter(name='params', type='group', children=params)
        parameter_tree.setParameters(parameter, showTop=False)
#}}}

def transmit():
    print "transmit pressed"

def write_rom():
    print "write_rom"

def read_rom():
    print "read_rom"

def load_file():
    print "load_file"

def save_file():
    print "save_file"

qt_mav.param_value_received.connect(refresh_table)
tuner.buttonRefersh.clicked.connect(qt_mav.refresh)
tuner.buttonTransmit.clicked.connect(transmit)
tuner.buttonWriteRom.clicked.connect(write_rom)
tuner.buttonReadRom.clicked.connect(read_rom)
# tuner.buttonRefersh.pressed.connect(exit())

tuner.show()

sys.exit(app.exec_())
