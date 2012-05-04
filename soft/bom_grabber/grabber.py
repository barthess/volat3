#!/usr/bin/python
# -*- coding: utf8 -*-

import urllib2
import re
import math


def __save_debug(page, name): #{{{ write debug page for manual investigation
    f = open(name, "wb")
    f.write(page)
    f.close
    #}}}


def __error_note(errmsg, log):#{{{ Notify user and make error record
    print errmsg
    #TODO: remove hardcoded file name
    # print "\tCheck error.log file"
    log.write(errmsg + '\n')
    #}}}


def __update_regular(row, index, st, log, page):#{{{ update non critical fields
    """ index - string index in dictionary
        st - string received from supplyer """

    if st == "":
        name = "dbg_" + row['VID'] + "_" + row['Vendor Part Num'] + ".html"
        __save_debug(page, name)
        print "*** WARNING! <<" + index + ">> not found for " + row['Mfg Part Num']

    if row[index] == "":
        row[index] = st

    elif row[index] != st:
        errmsg = "*** WARNING! " + index + " altered from <<" + row[index] + ">> to <<" + st + ">>"
        __error_note(errmsg, log)
        row[index] = st

    print "\t" + index + ": <<" + st + '>>'
    #}}}


def farnell(row, log): #{{{
    """ Grabber from Farnell site
    row - string from DB
    log - file for saving warning and error messages """

    # multiply to this currency to get USD
    currency = 2.0

    # Open an page url
    url = urllib2.urlopen("http://uk.farnell.com/" + row["Vendor Part Num"])
    page = url.read()
    # __save_debug(page, "test.html")
    # exit()
    # create empty temporal line
    megaline = ''

    # delete all new line symbols
    for i in page:
        if (i != '\n') and (i != '\r'):
            megaline += i

    # Is partnum exist on supplyer?
    __save_debug(page, "test.html")
    if re.match('^.*>[0-9]*</span> Product results found for', megaline) != None:
        errmsg = "*** ERROR! Product with " + row["Vendor Part Num"] + " not found on Farnell"
        __error_note(errmsg, log)
        return None
    else:
        pass

    # check manufacturer part number
    mfgpartnum = re.sub('^.*<dt>Manufacturer Part No:</dt><dd>', '', megaline)
    mfgpartnum = re.sub('</.*$', '', mfgpartnum)
    if mfgpartnum != row["Mfg Part Num"]:
        errmsg = "*** ERROR! Product part number in DB (" + row["Mfg Part Num"] + ") is not equal to supplyer partnum (" + mfgpartnum + ")"
        __error_note(errmsg, log)
        return None
    else:
        print "\tManufacturer part num: <<" + mfgpartnum + '>>'

    # Description just copied without changes if field is empty
    description = re.sub('^.*<meta name="description" content="[^,]*,[^,]*,', '', megaline)
    description = re.sub('" />.*$', '', description)
    if row['Description (bom-ex)'] == "":
        row['Description (bom-ex)'] = description
    print "\tDescription: <<" + description + '>>'

    # manufacturer name
    manufacturer = re.sub('^.*<dt>Manufacturer:</dt><dd style="text-align: left;">', '', megaline)
    manufacturer = re.sub('<.*$', '', manufacturer)
    __update_regular(row, 'Mfg Name', manufacturer, log, page)

    # Country of Origin
    origin = re.sub('^.*<strong> Country of Origin: </strong>[ ]*[A-Z]{2}[ ]*', '', megaline)
    origin = re.sub('<.*$', '', origin)
    __update_regular(row, 'Country of Origin', origin, log, page)

    # get price of one piece and covert it to $
    price = re.sub('^.*<input type="hidden" name="unitPrice" value="£','', megaline)
    price = re.sub('"/>.*$', '', price)
    price_usd = currency * float(price)
    price_usd_st = "$" + str(price_usd)
    __update_regular(row, 'Unit Price', price_usd_st, log, page)

    return row
#}}}



