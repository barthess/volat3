#!/usr/bin/python
# -*- coding: utf-8 -*-

import ConfigParser
import socket
import time
import datetime
import serial
from binascii import hexlify
import threading

from utils import *
import globalflags
flags = globalflags.flags

#
errlog = open('logs/proxy.log', 'a')

# load settings from config file
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

# serial port
bufsize  = config.getint('Serial', 'bufsize')
baudrate = config.getint('Serial', 'baudrate')
serport  = config.get('Serial', 'port')
ser      = serial.Serial(serport, baudrate, timeout = 0.5)

class SerialReader(threading.Thread):#{{{
    """ Infinitely read data from serial port and write it to all registered ports """
    # TODO: write all bytes to logfile
    def __init__(self, dev):
        threading.Thread.__init__(self)
        self.__stop = threading.Event()
        self.ser = dev
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.portlist = []
        for port in config.items("SocketOut"):
            self.portlist.append(config.getint("SocketOut", port[0]))

    def stop(self):
        self.__stop.set()

    def run(self):
        while True:
            if self.__stop.is_set():
                print "SerialProxy: reader exiting"
                return
            c = self.ser.read(bufsize)
            if len(c) != 0:
                for port in self.portlist:
                    self.sock.sendto(c, ("localhost", port))
    #}}}
class SerialWriter(threading.Thread):#{{{
    def __init__(self, dev):
        threading.Thread.__init__(self)
        self.__stop = threading.Event()
        self.ser = dev
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.5)
        p = config.getint("SocketIn", "PORT_UDP_SERPROXY")
        self.sock.bind(("localhost", p))

    def stop(self):
        self.__stop.set()

    def run(self):
        cin = ""
        while True:
            if self.__stop.is_set():
                print "SerialProxy: writer exiting"
                return
            # cin = self.sock.recv(1024)
            try: cin = self.sock.recv(1024)
            except socket.timeout: pass
            if len(cin) > 0:
                self.ser.write(cin)
                print hexlify(cin)
                print cin
                cin = ""
    #}}}

def main():
    reader = SerialReader(ser)
    writer = SerialWriter(ser)
    reader.start()
    writer.start()
    while True:
        try:
            time.sleep(0.5)
        except KeyboardInterrupt:
            print "Keyboard Interrupt caught"
            reader.stop()
            writer.stop()
            reader.join()
            writer.join()
            print "SerialProxy: stopped"
            return

main()




