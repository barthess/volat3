##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O0 -ggdb -fno-strict-aliasing -fomit-frame-pointer -falign-functions=16 -std=gnu99
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FWLIB),)
  USE_FWLIB = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = ch

# Imported source files and paths
CHIBIOS = ../../../chibios_dynclk
include $(CHIBIOS)/os/hal/platforms/STM32F1xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F1xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk
#include $(CHIBIOS)/boards/OLIMEX_STM32_103STK/board.mk
#include $(CHIBIOS)/test/test.mk

# Define linker script file here
LDSCRIPT= $(PORTLD)/STM32F103xD.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(TESTSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(CHIBIOS)/os/various/shell.c \
       $(CHIBIOS)/os/various/chprintf.c \
       ../eeprom/eeprom.c \
       ../eeprom/eeprom_testsuit.c \
       ../microrl/src/microrl.c \
       main.c \
       board.c \
       src/autopilot.c \
       src/message.c \
       src/utils.c \
       src/stubs.c \
       src/sanity.c \
       src/i2c_local.c \
       src/storage.c \
       src/sensors/mavsender.c \
       src/sensors/sensors.c \
       src/sensors/sensors_cmd.c \
       src/sensors/adc_local.c \
       src/sensors/discrete.c \
       src/sensors/spi_local.c \
       src/sensors/freq.c \
       src/exti_local.c \
       src/parameters/param.c \
       src/parameters/param_cli.c \
       src/parameters/persistant.c \
       src/link/usb_local.c \
       src/link/uart_local.c \
       src/link/cli.c \
       src/link/can_usb.c \
       src/link/can_local.c \
       src/link/cli_cmd.c \
       src/link/link.c \
       src/link/linkmgr.c \
       src/link/link_sortin.c \
       src/link/link_sortout.c \
       src/dsp/dsp.c \
       

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

# Dir with mavlink headers
MAVLINCDIR = ../mavlink/C

SUBMODULEINCDIR = ../eeprom ../microrl

INCDIR = $(PORTINC) $(KERNINC) $(TESTINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) \
         $(CHIBIOS)/os/various \
         $(MAVLINKDIR) \
         include \
         src/sensors \
         src/eeprom \
         src/link \
         src/dsp \
         src/parameters \
         $(SUBMODULEINCDIR) \
         $(MAVLINCDIR)/oblique \

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m3 -lm

#TRGT = arm-elf-
TRGT = arm-none-eabi-
#CC   = ccache $(TRGT)gcc
#CPPC = ccache $(TRGT)g++
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

ifeq ($(USE_FWLIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  CSRC += $(STM32SRC)
  INCDIR += $(STM32INC)
  USE_OPT += -DUSE_STDPERIPH_DRIVER
endif

include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk

# User difined rules here
MAKE_ALL_RULE_HOOK: res_usage
	
res_usage: $(BUILDDIR)/$(PROJECT).bin $(BUILDDIR)/$(PROJECT).elf \
			$(BUILDDIR)/$(PROJECT).hex $(BUILDDIR)/$(PROJECT).dmp
	./res_usage.sh	
