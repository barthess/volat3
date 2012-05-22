#! /usr/bin/env python

#
# EagleCAD parts manipulator
# Turns the exported partslist from EagleCad into a CSV 
# which has been arranged by parts of similar value and package 
# to facilitate ordering
#
# Usage eagle_parts [outputlist.csv] inputlist.txt
#   if output list is not specified, partslist.csv is written 
#
# Copyright (C) 2011 Andy Bardagjy. 
#
# This script is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This script is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
# eagle_parts
# Andy Bardagjy 7/13/2011
#

import sys
import os

outlist = ""
inlist = ""

# Parse the input arguments
if len(sys.argv) == 2:
    outlist = 'partslist.csv'
    inlist = sys.argv[1]
elif len(sys.argv) == 3:
    outlist = sys.argv[1]
    inlist = sys.argv[2]
else:
    print "Incorrect syntax, eagle_parts [outputlist.csv] inputlist.txt"
    sys.exit(2)

# Read in the file
ff = open(inlist, 'r')
lines = ff.readlines()

# In EAGLE files the column headings are on line 7 (v4 and v5)
# and on line 9 (v6)
headings = lines[8].split()
headinds = [0] # Indexes of the headings

for head in headings[1:]:
    headinds.append(lines[6].find(head))

headinds.append(-1)

# For each line, get the elements 
outp = []
tpart = []

line = ""
for line in lines[8:]:
    for index in range(len(headinds)-1):
        tele = line[headinds[index]:headinds[index+1]].strip()
        tpart.append(tele)
    outp.append(tpart)
    tpart=[]

# Find and count the unique parts
out = []
tpart = []
blist = []

for I, L in enumerate(outp):
    if I not in blist:
        tpart.append('1')
        tpart.append(L[headings.index("Value")])
        tpart.append(L[headings.index("Package")])
        tpart.append(L[headings.index("Part")])
        blist.append(I)
        out.append(tpart)
        for J, L2 in enumerate(outp):
            if J not in blist:
                if L2[headings.index("Value")] == tpart[1]:
                    if L2[headings.index("Package")] == tpart[2]:
                        tpart[0] = str(int(tpart[0]) + 1)
                        tpart[3] = tpart[3] + ' ' + L2[headings.index("Part")]
                        blist.append(J)
    tpart = []

# Format for Digikey: Quant, DK# (empty), Internal (Value, Package Part)
csvo = "Quantity, Digikey #, Internal #\n"
for line in out:
    csvo = csvo + line[0]+', ,'+line[1]+' '+line[2]+' '+line[3]+'\n'

# print list
ol = open(outlist, 'w')
ol.write(csvo)
ol.close()
