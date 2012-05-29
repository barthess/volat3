#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import serial
import os
import signal
import time
import argparse
import ConfigParser
from struct import pack
from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

import pygame
from pygame.locals import *
from gloss import *

# самопальные модули
from dsp import *
from localconfig import *
import telemetry
import log
import link
import tuner

# глобальные переменные между модулями
import globalflags

# sys.dont_write_bytecode = True # не компилировать исходники

# Очереди сообщений
q_log  = Queue(1) # записывальщика лога
q_tlm  = Queue(1) # для телеметрии
q_in   = Queue(8) # для пакетов с увву
q_out  = Queue(8) # для пакетов на увву
q_cal  = Queue(1) # для калбировочных коэффициетов на увву

# события для блокирования до тех пор, пока событие в состоянии clear
e_pause = Event() # лок для постановки процесса на паузу
e_pause.clear()

e_kill = Event() # предложение умереть добровольно
e_kill.clear()


def main(q_tlm, config):
    """ Запускает отрисовку телеметрии и пробрасывает в нее очередь сообщений. """
    tlm = telemetry.Telemetry("MOSK - Mobile Operational System Kamikaze")
    Gloss.screen_resolution = 1024,768
    #Gloss.full_screen = True
    #Gloss.enable_multisampling = False
    Gloss.enable_multisampling = True
    #pygame.mouse.set_visible(False)
    pygame.mouse.set_visible(True)
    tlm.init(q_tlm)
    tlm.run()


# запускатор процессов
if __name__ == '__main__':
    freeze_support()

    # command line parser
    parser = argparse.ArgumentParser(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            # Usage text
            description=(''' This is stub for command prompt help. '''))
    parser.add_argument('input_file',
            metavar='filename',
            nargs='?', # аргумент необязателен
            type=file,
            help='path to telemetry log file')
    args = parser.parse_args()

    # загрузим конфиг. Позднее мы его передадим каждому нуждающемуся процессу.
    config = ConfigParser.SafeConfigParser()
    config.read('default.cfg')

    # главная программная вилка.
    # Определяет, читаем мы телеметрию из лога, или из устройства
    e_pause.clear()
    e_kill.clear()

    p_main = Process(target=main, args=(q_tlm, config))
    p_main.start()

    if args.input_file != None:
        p_logreader = Process(target=log.play,
                                  args=(args.input_file.name, q_tlm, e_pause, e_kill, ))
        p_logreader.start()
    else:
        pass
        p_linkin = Process(target=link.linkin,
                             args=(q_tlm, q_log, e_pause, e_kill, config))
        p_linkin.start()

        #p_linkout = Process(target=link.linkout,
        #                      args=(q_out, e_pause, e_kill))
        #p_linkout.start()

        p_logwriter = Process(target=log.record,
                              args=(q_log, e_pause, e_kill, ))
        p_logwriter.start()



    time.sleep(4) # ждем, пока все процессы подхватятся
    print "--- clear global pause"
    e_pause.set() # снимаем с паузы порожденные процессы

    p_main.join() # тусим тут, пока главный процесс не выйдет
    print "Telemetry joined"
    e_kill.set()  # предлагаем всем остальным выйти

    time.sleep(1)
    try:
        if p_protomanager != None: p_protomanager.join()
    except:
        pass

    try:
        if p_link != None: p_link.join()
    except:
        pass

    try:
        if p_logwriter != None: p_logwriter.join()
        print "killed"
    except:
        pass


os.kill(os.getpid(), signal.SIGABRT)

