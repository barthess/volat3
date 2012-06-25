#!/usr/bin/python
# -*- coding: utf-8 -*-

from pyqtgraph.Qt import QtCore, QtGui


class RelCheckboxGrid(QtGui.QWidget):#{{{
    """ """

    changed = QtCore.pyqtSignal(int)

    def __init__(self, parent):
        # super(RelCheckboxGrid, self).__init__(parent=parent)
        QtGui.QWidget.__init__(self, parent=parent)
        self.grid = [] # array of checkbox objects

    def addCheckbox(self, i, st):
        step = 16
        x0 = 5
        y0 = 25
        self.grid.append(QtGui.QCheckBox(parent=self,
                        text=QtCore.QString.fromUtf8(st),
                        geometry=QtCore.QRect(x0, y0 + i * step, 350, 20)))
        self.grid[i].clicked.connect(self._emitState)

    def setState(self, state):
        i = 0
        while i < len(self.grid):
            self.grid[i].setChecked((state >> i) & 1)
            i += 1

    def getState(self):
        i = 0
        state = 0
        while i < len(self.grid):
            if self.grid[i].checkState():
                state += 1 << i
            i += 1
        return state

    def _emitState(self):
        self.changed.emit(self.getState())
#}}}



