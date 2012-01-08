#!/usr/bin/python
# -*- coding: cp1251 -*-

import sys
import os
import xml.etree.ElementTree
import time


def kill(subtree, strid):
    """ Удоляет ненужные объекты согласно словарю
    Принимает:
        дерево
        id удаляемого объекта"""

    def delnode(i, strid):
        if i.attrib['id'] == strid:
            subtree.remove(i)

    for i in subtree:
        if i.tag[-1:] == "g":
            delnode(i, strid)
        elif i.tag[-4:] == "path":
            delnode(i, strid)

        # recursive call
        kill(i, strid)


def generate(lst, name):
    """создает новую svg путем удаления элементов согласно списка"""
    tmp = []
    for i in lst:
        tmp.append(i)
    try: tmp.remove(name)
    except ValueError: pass

    tree = xml.etree.ElementTree.parse(open("tiers.svg"))
    for i in tmp:
        kill(tree._root, i)

    newfile = open(name + '.svg', 'w')
    print "save" + name + '.svg'
    tree.write(newfile, "UTF-8")


# массивы всех элементов сразу. Мы из них будем исключать
# те элементы, которые должны остаться
tier = ["leg1_red", "leg2_red", "leg3_red",
        "leg4_red", "leg5_red", "leg6_red"]

sygns = ["autriggers_sygn_red", "autriggers_sygn_grey",
         "tiers_sygn_grey","tiers_sygn_red"]

bodie = ["axle_1","axle_2","axle_3", "shaft",
         "leg1_grey", "leg2_grey","leg3_grey",
         "leg4_grey","leg5_grey","leg6_grey"]


tmp = ["axle_2", "leg2_grey", "leg5_grey"] + sygns + tier
generate(tmp, "autriggers_bg")

tmp = sygns + tier
generate(tmp, "tiers_bg")

tmp = tier + sygns + bodie
generate(tmp, "autriggers_sygn_red")
generate(tmp, "autriggers_sygn_grey")
generate(tmp, "tiers_sygn_red")
generate(tmp, "tiers_sygn_grey")
generate(tmp, "leg1_red")
generate(tmp, "leg2_red")
generate(tmp, "leg3_red")
generate(tmp, "leg4_red")
generate(tmp, "leg5_red")
generate(tmp, "leg6_red")

