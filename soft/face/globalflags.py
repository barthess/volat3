#!/usr/bin/python
# -*- coding: utf-8 -*-

# '''
# This values moved here from *.cfg for ability of changing from elsewhere
# '''

from multiprocessing import Lock

flags = {"lock" : Lock(), # для атомарного доступа к флагам
         "atm_mode" : False, # Режим настройки дрессированной обезьяной
         "debug" : False, # включает отладочный режим
         "mouse_capture" : False, # включает захват координат курсора в качестве входных данных
         "help_flag" : False,# если взведен -- показывать экран помощи
         "logreplay_flag" : False, # если взведен -- вывести на экран значок play
         "connection_interrupted" : False, # связь с мпиовд прервалась
         }
