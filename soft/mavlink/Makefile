# Process this file with GNU make
.PHONY: all mavlink_c mavlink_py clean

TOP_DIR = $(shell pwd)
MAVGEN_DIR = ../../../mavlink
# fixed files from mavlink bundle
FIXED_PY = mavutil.py mavextra.py

all: mavlink_c mavlink_py

mavlink_c: $(TOP_DIR)/C/mpiovd/mpiovd.h

mavlink_py: $(TOP_DIR)/python/mavlink.py $(FIXED_PY)

# C headers
$(TOP_DIR)/C/mpiovd/mpiovd.h: $(TOP_DIR)/definitions/mpiovd.xml
	cp $(TOP_DIR)/definitions/mpiovd.xml $(MAVGEN_DIR)/message_definitions/v1.0
	$(MAVGEN_DIR)/pymavlink/generator/mavgen.py --lang=C --wire-protocol=1.0 -o C $(MAVGEN_DIR)/message_definitions/v1.0/mpiovd.xml
	rm $(MAVGEN_DIR)/message_definitions/v1.0/mpiovd.xml

# python bindings
$(TOP_DIR)/python/mavlink.py: $(TOP_DIR)/definitions/mpiovd.xml
	cp $(TOP_DIR)/definitions/mpiovd.xml $(MAVGEN_DIR)/message_definitions/v1.0
	$(MAVGEN_DIR)/pymavlink/generator/mavgen.py --lang=python --wire-protocol=1.0 -o python/mavlinkv10.py $(MAVGEN_DIR)/message_definitions/v1.0/mpiovd.xml
	rm $(MAVGEN_DIR)/message_definitions/v1.0/mpiovd.xml

# fixed files for python bingings
$(FIXED_PY): $(TOP_DIR)/definitions/mpiovd.xml
	cd $(MAVGEN_DIR)/pymavlink; cp $(FIXED_PY) $(TOP_DIR)/python

clean:
	rm -rf $(TOP_DIR)/python/*.pyc
	rm -rf $(TOP_DIR)/python/*.py
	rm -rf $(TOP_DIR)/C/mpiovd
	rm -rf $(TOP_DIR)/C/common
	rm -rf $(TOP_DIR)/C/*.h
	rm -rf $(TOP_DIR)/C/*.hpp
