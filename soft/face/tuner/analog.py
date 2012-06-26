#!/usr/bin/python
# -*- coding: utf-8 -*-

from pyqtgraph.Qt import QtCore, QtGui

import sys
import os
import time
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../../mavlink/python'))
import mavlink
import mavutil

class VolatAnWidget(QtGui.QWidget):#{{{
    """ Tuner for single analog channel """

    valueChanged = QtCore.pyqtSignal(str, int)

    def __init__(self, name, parent):
        QtGui.QWidget.__init__(self, parent=parent)
        self.c1 = 0
        self.c2 = 0
        self.c3 = 0
        L = 25
        H = 20
        space = 5
        x = 0
        self.label = QtGui.QLabel(parent=self, text=name)
        self.label.setGeometry(QtCore.QRect(x, 0, L, H))

        x += space + L
        L = 40
        self.comboFilterLen = QtGui.QComboBox(self)
        self.comboFilterLen.setGeometry(QtCore.QRect(x, 0, L, H))
        self.comboFilterLen.addItem("0")
        self.comboFilterLen.addItem("4")
        self.comboFilterLen.addItem("8")
        self.comboFilterLen.addItem("16")
        self.comboFilterLen.addItem("32")
        self.comboFilterLen.addItem("64")

        x += space + L
        L = 50
        self.lineedit_c1 = QtGui.QLineEdit(parent=self)
        self.lineedit_c1.setGeometry(QtCore.QRect(x, 0, L, H))
        self.lineedit_c1.editingFinished.connect(self.slotValueChanged_c1)
        self.lineedit_c1.setValidator(QtGui.QIntValidator(self.lineedit_c1))

        x += space + L
        self.lineedit_c2 = QtGui.QLineEdit(parent=self)
        self.lineedit_c2.setGeometry(QtCore.QRect(x, 0, L, H))
        self.lineedit_c2.editingFinished.connect(self.slotValueChanged_c2)
        self.lineedit_c2.setValidator(QtGui.QIntValidator(self.lineedit_c2))

        x += space + L
        self.lineedit_c3 = QtGui.QLineEdit(parent=self)
        self.lineedit_c3.setGeometry(QtCore.QRect(x, 0, L, H))
        self.lineedit_c3.editingFinished.connect(self.slotValueChanged_c3)
        self.lineedit_c3.setValidator(QtGui.QIntValidator(self.lineedit_c3))

        x += space + L
        L = 30
        self.label_raw = QtGui.QLabel(parent=self, text='4096')
        self.label_raw.setGeometry(QtCore.QRect(x, 0, L, H))

        x += space + L
        self.label_scaled = QtGui.QLabel(parent=self, text='123.7')
        self.label_scaled.setGeometry(QtCore.QRect(x, 0, L, H))

        x += 2 * space + L
        self.buttonPlot = QtGui.QPushButton(parent=self, text='P')
        self.buttonPlot.setGeometry(QtCore.QRect(x, 0, 20, 20))

    def slotValueChanged_c1(self):
        n = int(self.lineedit_c1.text())
        self.valueChanged.emit("c1", n)

    def slotValueChanged_c2(self):
        n = int(self.lineedit_c2.text())
        self.valueChanged.emit("c2", n)

    def slotValueChanged_c3(self):
        n = int(self.lineedit_c3.text())
        self.valueChanged.emit("c3", n)
#}}}

class VolatAnGrid(QtGui.QWidget):#{{{

    def __init__(self, indevice, parent=None):
        """
        parent -- parent widget
        indevice -- device to send output data
        """
        self.indevice = indevice
        # parent
        QtGui.QWidget.__init__(self, parent=parent)
        self.grid = []
        H = 20
        x = 0
        y = 20
        space = 3
        i = 0
        while i < 16:
            an = VolatAnWidget(name="AN" + str(i + 1), parent=self)
            an.setGeometry(QtCore.QRect(space, y + space*i + H*i, 400, H))
            self.grid.append(an)
            i += 1

    def slotParamMsg(self, msg):
        """ msg -- mavlink message """
        # print msg.param_value
        paramtype = msg.param_id.split('_')[0]
        if paramtype == "AN":
            ch = msg.param_id.split('_')[1][2:]
            ch = int(ch) - 1
            v = int(round(msg.param_value))

            # print msg.param_id.split('_')[2][1]
            print msg.param_id.split('_')[2][1]
            if msg.param_id.split('_')[2][1] == "1":
                self.grid[ch].lineedit_c1.setText(str(v))
            elif msg.param_id.split('_')[2][1] == "2":
                self.grid[ch].lineedit_c2.setText(str(v))
            elif msg.param_id.split('_')[2][1] == "3":
                self.grid[ch].lineedit_c3.setText(str(v))

    def transmit(self):
        inmaster = mavutil.mavlink_connection(self.indevice, input=False)
        inmaster.target_system = 20

        i = 0
        for an in self.grid:
            param_id = "AN_ch" + str(i + 1) + "_c1"
            v = int(self.grid[i].lineedit_c1.text())
            inmaster.mav.param_set_send(20, 0, param_id, v, 6)
            time.sleep(0.05)

            param_id = "AN_ch" + str(i + 1) + "_c2"
            v = int(self.grid[i].lineedit_c2.text())
            inmaster.mav.param_set_send(20, 0, param_id, v, 6)
            time.sleep(0.05)

            param_id = "AN_ch" + str(i + 1) + "_c3"
            v = int(self.grid[i].lineedit_c3.text())
            inmaster.mav.param_set_send(20, 0, param_id, v, 6)
            time.sleep(0.05)

            i += 1
            print "saved AN", i

        # param_id = "AN_ch" + str(i + 1) + "_c3"
        # v = int(self.grid[i].lineedit_c3.text())
        # inmaster.mav.param_set_send(20, 0, param_id, v, 6)
        # time.sleep(0.05)

        inmaster.close()
#}}}


