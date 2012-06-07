#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
import sys
import time
import collections
import user

from Queue import Empty # для отлова исключений
from multiprocessing import Queue, Event

from pyqtgraph.Qt import QtCore, QtGui
import pyqtgraph.parametertree.parameterTypes as pTypes
from pyqtgraph.parametertree import Parameter, ParameterTree, ParameterItem, registerParameterType
from PyQt4 import uic

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



def refresh():#{{{
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
sys.exit(app.exec_())
