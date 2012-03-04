#!/usr/bin/python
# -*- coding: cp1251 -*-



def trapezoidal(f):
    a = 0
    b = dt
    I = (b - a) * (f[0] + f[1]) * 0.5
    return I

def simpson_quadratic(f):
    a = 0
    b = dt * 2
    I = ((b - a) / 6.0) * (f[0] + 4*f[1] + f[2])
    return I

def simpson_cubic(fa, fm1, fm2, fb):
    a = 0
    b = dt * 3
    I = ((b - a) / 8.0) * (fa + 3*fm1 + 3*fm2 + fb)
    return I




# кольцевой буфер для нужд трудящихся
class RingBuffer:
    def __init__(self, size):
        self.data = [0.0 for i in xrange(size)]
        self.summ = 0.0

    def append(self, x):
        self.summ += x
        self.summ -= self.data[0]
        self.data.pop(0)
        self.data.append(x)

    def get_summ(self):
        return self.summ

    def get_avg(self):
        return self.summ / len(self.data)
