#!/usr/bin/python
# -*- coding: cp1251 -*-

import sys
import serial
import os
import time
import argparse
import ConfigParser
from struct import pack
from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

import pygame
from pygame.locals import *
from gloss import *

# ����������� ������
from dsp import *
from localconfig import *
import telemetry
import log
import link
import tuner

# ���������� ���������� ����� ��������
import globalflags

sys.dont_write_bytecode = True # �� ������������� ���������

# ������� ���������
q_log  = Queue(1) # �������������� ����
q_tlm  = Queue(1) # ��� ����������
q_in   = Queue(8) # ��� ������� � ����
q_out  = Queue(8) # ��� ������� �� ����
q_cal  = Queue(1) # ��� ������������� ������������ �� ����

# ������� ��� ������������ �� ��� ���, ���� ������� � ��������� clear
e_pause = Event() # ��� ��� ���������� �������� �� �����
e_pause.clear()

e_kill = Event() # ����������� ������� �����������
e_kill.clear()


def main(q_tlm, config):
    """ ��������� ��������� ���������� � ������������ � ��� ������� ���������. """
    tlm = telemetry.Telemetry("MOSK - Mobile Operational System Kamikaze")
    Gloss.screen_resolution = 1024,768
    #Gloss.full_screen = True
    #Gloss.enable_multisampling = False
    Gloss.enable_multisampling = True
    #pygame.mouse.set_visible(False)
    pygame.mouse.set_visible(True)
    tlm.init(q_tlm)
    tlm.run()


# ���������� ���������
if __name__ == '__main__':
    freeze_support()

    # command line parser
    parser = argparse.ArgumentParser(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            # Usage text
            description=(''' This is stub for command prompt help. '''))
    parser.add_argument('input_file',
            metavar='filename',
            nargs='?', # �������� ������������
            type=file,
            help='path to telemetry log file')
    args = parser.parse_args()

    # �������� ������. ������� �� ��� ��������� ������� ������������ ��������.
    config = ConfigParser.SafeConfigParser()
    config.read('default.cfg')

    # ������� ����������� �����.
    # ����������, ������ �� ���������� �� ����, ��� �� ����������
    e_pause.clear()
    e_kill.clear()

    p_main = Process(target=main, args=(q_tlm, config))
    p_main.start()

    if args.input_file != None:
        p_logreader = Process(target=log.play,
                                  args=(args.input_file.name, q_tlm, e_pause, e_kill, ))
        p_logreader.start()
    else:
        p_linkin = Process(target=link.linkin,
                             args=(q_tlm, q_log, e_pause, e_kill, config))
        p_linkin.start()

        #p_linkout = Process(target=link.linkout,
        #                      args=(q_out, e_pause, e_kill))
        #p_linkout.start()

        p_logwriter = Process(target=log.record,
                              args=(q_log, e_pause, e_kill, ))
        p_logwriter.start()



    time.sleep(4) # ����, ���� ��� �������� �����������
    print "--- clear global pause"
    e_pause.set() # ������� � ����� ����������� ��������

    p_main.join() # ���� ���������� �������� ��������
    e_kill.set()  # ���������� ���� ��������� �����

    time.sleep(0.5)
    # http://metazin.wordpress.com/2008/08/09/how-to-kill-a-process-in-windows-using-python/
    os.system("taskkill /im python.exe /f")
    p_protomanager.join()
    p_link.join()
    p_logwriter.join()


