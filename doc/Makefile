# Process this file with GNU make

TOP_DIR = .
OUTPUT_DIR=$(TOP_DIR)/output

SUBDIRS = documents/component_parts/uvvu/pcb_e3\
		 documents/component_parts/uvvu/pcb_pe3\
		 documents/component_parts/uvvu/dev_e4\
		 documents/component_parts/uvvu/dev_pe4\

#include $(TOP_DIR)/include.mak

all:  all-recursive
all-recursive: ./*
	for i in $(SUBDIRS); do $(MAKE) -C $$i all || exit $$?; done


allbw:  allbw-recursive
allbw-recursive: ./*
	for i in $(SUBDIRS); do $(MAKE) -C $$i allbw || exit $$?; done

pdf: pdf-recursive
pdf-recursive: ./*
	for i in $(SUBDIRS); do $(MAKE) -C $$i pdf || exit $$?; done

distrib: distrib-recursive
distrib-recursive: ./*
	for i in $(SUBDIRS); do $(MAKE) -C $$i distrib || exit $$?; done


reallyall: all allbw

install: install-recusive
install-recusive:
	for i in $(SUBDIRS); do $(MAKE) -C $$i install || exit $$?; done

clean: clean-recursive
	-rm -f *.origin *.bak *.log *~
	-cd tex && rm -f *.origin *.bak *.log *~
	-cd scripts && rm -f *.origin *.bak *.log *~

distclean: clean-recursive
	for i in $(SUBDIRS); do $(MAKE) -C $$i distclean || exit $$?; done
	-rm -f $(OUTPUT_DIR)/*.pdf

clean-recursive:
	for i in $(SUBDIRS); do $(MAKE) -C $$i clean || exit $$?; done
