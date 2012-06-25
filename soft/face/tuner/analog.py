#!/usr/bin/python
# -*- coding: utf-8 -*-

from pyqtgraph.Qt import QtCore, QtGui


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

    def __init__(self, parent):
        parent
        QtGui.QWidget.__init__(self, parent=parent)
        self.grid = []
        H = 20
        x = 0
        space = 3
        i = 0
        while i < 10:
            an = VolatAnWidget(name="AN" + str(i + 1), parent=self)
            an.setGeometry(QtCore.QRect(space, space*i + H*i, 400, H))
            self.grid.append(an)
            i += 1


#}}}
