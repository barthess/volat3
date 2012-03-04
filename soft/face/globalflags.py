#!/usr/bin/python
# -*- coding: utf-8 -*-

from multiprocessing import Lock

flags = {"lock" : Lock(), # для атомарного доступа к флагам
         "atm_mode" : False, # Режим настройки дрессированной обезьяной
         "debug" : True, # включает отладочный режим
         "help_flag" : False,# если взведен -- показывать экран помощи
         "logreplay_flag" : False, # если взведен -- вывести на экран значок play
         }
