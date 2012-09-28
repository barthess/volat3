#!/usr/bin/python
# -*- coding: utf-8 -*-

from math import *

import numpy as np
import matplotlib.pyplot as plt

global R0
global R1
global R2

R0 = 1000.0
R1 = 100000.0
R2 = 10000.0

def Ud(U2):
    return 0.037 * np.log(250*U2 + 1)

def U1(U2):
    pass

t1 = np.arange(0, 1000, 0.1)

U2real = np.array([0, 0.019, 0.111, 0.218, 0.829, 1.245])
Udreal = np.array([0, 0.027, 0.124, 0.150, 0.191, 0.214])

t2 = np.arange(0, 1.5, 0.01)

plt.ylabel("Ud")
plt.xlabel("U2")
plt.plot(t2, Ud(t2), U2real, Udreal, 'ro')

plt.show()

# plt2.plot(t1, U(t1))
# plt2.show()
