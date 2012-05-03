#!/usr/bin/python
# -*- coding: utf8 -*-

import urllib2
import re
import csv

fieldnames = ['Mfg Part Num','Mfg Name','VID','Vendor Part Num','Unit Price','Description (bom-ex)','Country of Origin','Customer Ref']


# Open our db as csv
dbReader = csv.DictReader(open('partsdb.tsv', 'rb'), delimiter='\t')
dbWriter = csv.DictWriter(open('partsdb_new.tsv', 'wb'), fieldnames, delimiter='\t')
dbWriter.writeheader()




def farnellgrab(row):

    # multiply to this currency to get USD
    currency = 2.0

    # Open an page url
    page = urllib2.urlopen("http://uk.farnell.com/" + row["Vendor Part Num"])
    # empty temporal line
    megaline = ''

    # delete all new line symbols
    for i in page.read():
        if i != '\n' and i != '\r':
            megaline += i

    #TODO: check mfg part num

    description = re.sub('^.*<meta name="description" content="[^,]*,[^,]*,', '', megaline)
    description = re.sub('" />.*$', '', description)
    print "Description: <<" + description + '>>'

    manufacturer = re.sub('^.*<dt>Manufacturer:</dt><dd style="text-align: left;">', '', megaline)
    manufacturer = re.sub('<.*$', '', manufacturer)
    print "Manufacturer: <<" + manufacturer + '>>'

    origin = re.sub('^.*<strong> Country of Origin: </strong>[A-Z][A-Z]', '', megaline)
    origin = re.sub('<.*$', '', origin)
    print "Country of Origin: <<" + origin + '>>'

    price = re.sub('^.*<input type="hidden" name="unitPrice" value="£','', megaline)
    price = re.sub('"/>.*$', '', price)
    price_usd = currency * float(price)
    print "Unit price: " + str(price_usd) + ' USD'

    print "----------------------------------------------------------------"
    if row['Description (bom-ex)'] == "":
        row['Description (bom-ex)'] = description
    row['Mfg Name'] = manufacturer
    row['Country of Origin'] = origin
    row['Unit Price'] = price_usd

    return row





for row in dbReader:
    print ""
    print "\t" + row["Mfg Part Num"]

    if row["VID"] == "Farnell":
        dbWriter.writerow(farnellgrab(row))

    elif row["VID"] == "Mouser":
        print "TODO: mouser" + row["Vendor Part Num"]
        dbWriter.writerow(row)

    elif row["VID"] == "Integral":
        print "TODO: integral" + row["Vendor Part Num"]
        dbWriter.writerow(row)

    elif row["VID"] == "BrownBear":
        print "TODO: brown bear" + row["Vendor Part Num"]
        dbWriter.writerow(row)

    elif row["VID"] == "":
        pass # just empty line

    else:
        print "I do not know how to deal with", row["VID"], "Keep as is."
        dbWriter.writerow(row)










