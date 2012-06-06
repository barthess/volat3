#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
import sys
import time

from Queue import Empty # для отлова исключений
from multiprocessing import Queue, Event

from PyQt4.QtCore import QObject
from PyQt4.QtGui import QApplication, QDialog
from PyQt4 import uic

def refresh():
    print "refresh clicked"

app = QApplication(sys.argv)

tuner = uic.loadUi("tuner.ui")
tuner.buttonRefersh.clicked.connect(refresh)


tuner.show()
sys.exit(app.exec_())
