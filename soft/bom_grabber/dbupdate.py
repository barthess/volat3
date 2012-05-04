#!/usr/bin/python
# -*- coding: utf8 -*-

import csv
import grabber
import argparse

# command line parser {{{
parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        # Usage text
        description=('''\
Place help here
'''))

parser.add_argument('inputfile',
        metavar='filename',
        type=file,
        help='path to database file')

args = parser.parse_args()
#}}}

#
fieldnames = ['Mfg Part Num','Mfg Name','VID','Vendor Part Num','Unit Price','Description (bom-ex)','Country of Origin','Customer Ref']

# Open our db as input csv, and output csv
dbReader = csv.DictReader(args.inputfile, delimiter='\t')
dbWriter = csv.DictWriter(open('partsdb_new.tsv', 'wb'), fieldnames, delimiter='\t')
dbWriter.writeheader()

# error log file
log = open("error.log", "wb")

# iterrate through DB and check every line
for row in dbReader:
    print "----------------------------------------------------------------"
    print row["Mfg Part Num"]

    if row["VID"] == "Farnell":
        result = grabber.farnell(row, log)
        if result != None:
            dbWriter.writerow(result)
        else:
            dbWriter.writerow(row)

    elif row["VID"] == "Mouser":
        log.write("TODO: mouser " + row["Vendor Part Num"] + "\n")
        dbWriter.writerow(row)

    elif row["VID"] == "Integral":
        log.write("TODO: Integral " + row["Vendor Part Num"] + "\n")
        dbWriter.writerow(row)

    elif row["VID"] == "BrownBear":
        log.write("TODO: BrownBear " + row["Vendor Part Num"] + "\n")
        dbWriter.writerow(row)

    elif row["VID"] == "":
        pass # just empty line

    else:
        print "I do not know how to deal with", row["VID"], "Keep as is."
        dbWriter.writerow(row)










