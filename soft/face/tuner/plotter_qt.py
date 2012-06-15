#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import ConfigParser
from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
from scipy.signal import lfilter
import pyqtgraph as pg

import link

# Очереди сообщений
q_plot  = Queue(32) # данные для отрисовки

# настройка и запуск процесса связи {{{
e_pause = Event() # лок для постановки процесса на паузу
e_pause.clear()
e_kill = Event() # предложение умереть добровольно
e_kill.clear()

# read socket settings settings from config
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

device = "udp:localhost:" + str(config.getint("SocketOut", "PORT_UDP_TUNER"))

p_linkin = Process(target=link.input, args=(q_plot, e_pause, e_kill, device, ))
p_linkin.start()
e_pause.set()
#}}}

SCOPE_LEN = 200 # количество точек на экране

import fir
taps = fir.get_taps()
print taps


app = QtGui.QApplication([])
win = pg.GraphicsWindow(title="Basic plotting examples")
# win.resize(1024,600)

p = win.addPlot(title="Updating p")

curve_raw      = p.plot(pen='w')
pen_filtered   = pg.mkPen('r', width=3)
curve_filtered = p.plot(pen=pen_filtered)

data = []
i = 0

def update():
    global curve_raw, curve_filtered, data, i
    while not q_plot.empty():
        m = q_plot.get_nowait()
        if m.get_type() == "MPIOVD_SENSORS_RAW":
            i += 1
            if i > SCOPE_LEN:
                data.pop(0)
            data.append(m.speed)

    curve_raw.setData(data)
    curve_filtered.setData(lfilter(taps, 1.0, data))

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(50)

## Start Qt event loop unless running in interactive mode or using pyside.
if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
    app.exec_()

e_kill.set()  # предлагаем выйти порожденным процессам
p_linkin.join()


