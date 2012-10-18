#!/usr/bin/python
# -*- coding: cp1251 -*-

from math import *
from scipy import interpolate, array, append

import matplotlib.pyplot as plt
import numpy as np

class linear: #{{{
    """ Класс для расчета значения путем аппроксимации характеристики датчика прямой
    y = kx + b
    """

    def __init__(self, x1, y1, x2 ,y2):
        """
        Принимает:
            координаты двух точек
        """
        x1 = float(x1)
        x2 = float(x2)
        y1 = float(y1)
        y2 = float(y2)
        self.k = (y2 - y1) / (x2 - x1)
        self.b = (x2*y1 - x1*y2) / (x2 - x1)

    def get(self, x):
        """
        Возвращает аппроксимированную точку

        Принимает:
            х -- координаты по оси Х
        """
        return self.k * x + self.b
#}}}
class interp1d:#{{{
    def __init__(self, x, y, x_min, x_max):
        x = array(x)
        y = array(y)
        x_min = float(x_min)
        x_max = float(x_max)

        # linearly extrapolate 2 additional points at begin and at the end
        if (x_min < x[0]):
            line_begin = linear(x[0], y[0], x[1], y[1])
            y_min = line_begin.get(x_min)
            x = append(x_min, x)
            y = append(y_min, y)

        if (x_max > x[-1]):
            line_end   = linear(x[-2], y[-2], x[-1], y[-1])
            y_max = line_end.get(x_max)
            y = append(y, y_max)
            x = append(x, x_max)

        self.f = interpolate.interp1d(x, y, kind='linear')

    def get(self, x):
        return self.f(x)
#}}}

