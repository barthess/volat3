#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import os
import signal
import time
import ConfigParser

from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

import pygame
from pygame.locals import *
from gloss import *

# самопальные модули
from utils import *
import telemetry
import link

# глобальные переменные между модулями
import globalflags
flags = globalflags.flags

# Очереди сообщений
q_tlm  = Queue(8) # для телеметрии

# события для блокирования до тех пор, пока событие в состоянии clear
e_pause = Event() # лок для постановки процесса на паузу
e_pause.clear()

e_kill = Event() # предложение умереть добровольно
e_kill.clear()


def main(q_tlm):
    """ Запускает отрисовку телеметрии и пробрасывает в нее очередь сообщений. """
    pygame.init()
    tlm = telemetry.Telemetry("MOSK - Mobile Operational System Kamikaze")
    Gloss.screen_resolution = 1024,768
    Gloss.full_screen = False
    # Gloss.enable_multisampling = False
    Gloss.enable_multisampling = True
    #pygame.mouse.set_visible(False)
    pygame.mouse.set_visible(True)
    tlm.init(q_tlm)
    tlm.run()


# запускатор процессов
if __name__ == '__main__':
    freeze_support()

    # read socket settings settings from config
    config = ConfigParser.SafeConfigParser()
    config.read('default.cfg')

    device = "udp:localhost:" + str(config.getint("SocketOut", "PORT_UDP_HUD"))

    # файл для записи ошибок
    errlog = open(config.get("Log", "log_hud"), 'a')

    # clear events just to be safe
    e_pause.clear()
    e_kill.clear()

    p_main = Process(target=main, args=(q_tlm,))
    p_main.start()

    # need only raw data for a moment
    p_linkin = Process(target=link.input, args=(q_tlm, e_pause, e_kill, device, ))
    p_linkin.start()

    time.sleep(1) # ждем, пока все процессы подхватятся
    dbgprint("**** clear global pause")
    e_pause.set() # снимаем с паузы порожденные процессы

    p_main.join() # тусим тут, пока главный процесс не завершится
    dbgprint("**** Telemetry process successfully exited.")
    e_kill.set()  # предлагаем всем остальным выйти

    p_linkin.join()

