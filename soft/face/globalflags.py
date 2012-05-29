#!/usr/bin/python
# -*- coding: utf-8 -*-

from multiprocessing import Lock

flags = {"lock" : Lock(), # для атомарного доступа к флагам
         "atm_mode" : False, # Режим настройки дрессированной обезьяной
         "debug" : True, # включает отладочный режим
         "mouse_capture" : True, # включает захват координат курсора в качестве входных данных
         "help_flag" : False,# если взведен -- показывать экран помощи
         "logreplay_flag" : False, # если взведен -- вывести на экран значок play
         }
