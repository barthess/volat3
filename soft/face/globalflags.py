#!/usr/bin/python
# -*- coding: cp1251 -*-

from multiprocessing import Lock

flags = {"lock" : Lock(), # ��� ���������� ������� � ������
         "atm_mode" : False, # ����� ��������� �������������� ���������
         "debug" : True, # �������� ���������� �����
         "help_flag" : False,# ���� ������� -- ���������� ����� ������
         "logreplay_flag" : False, # ���� ������� -- ������� �� ����� ������ play
         }
