#!/usr/bin/python
# -*- coding: utf-8 -*-

# глобальные переменные между модулями
import globalflags
flags = globalflags.flags

def dbgprint(st):
    if flags["debug"]:
        print st


