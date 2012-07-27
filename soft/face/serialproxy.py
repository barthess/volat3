#!/usr/bin/python
# -*- coding: utf-8 -*-

import ConfigParser
import socket
import time
import serial
from binascii import hexlify
import threading

from utils import *
import globalflags
flags = globalflags.flags

# load settings from config file
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

# serial port
bufsize  = config.getint('Serial', 'bufsize')
baudrate = config.getint('Serial', 'baudrate')
serport  = config.get('Serial', 'port')

print "trying to open", serport, "at", baudrate, "speed..."
ser = serial.Serial(serport, baudrate, timeout = 0.5)
print "  success!"

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
            print "  adding", port[1], "port to destination list"
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
        print "  binding writer to", p, "port"

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
                print "==>", hexlify(cin)
                print "==>", cin
                cin = ""
    #}}}
def main():#{{{
    print "starting reader thread"
    reader = SerialReader(ser)
    reader.start()
    print "starting writer thread"
    writer = SerialWriter(ser)
    writer.start()
    while True:
        try:
            time.sleep(0.5)
        except KeyboardInterrupt:
            print "\nKeyboard Interrupt caught"
            reader.stop()
            writer.stop()
            reader.join()
            writer.join()
            print "closing serial port"
            ser.close()
            print "SerialProxy: stopped"
            return
#}}}

main()

