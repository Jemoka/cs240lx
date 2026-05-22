# need the makefile to give an error if this doesn't exist
PROGS := 0-fill-screen.c
# PROGS := 1-draw-commands.c

COMMON_SRC := sh1106-display-driver.c
COMMON_SRC += standard-ascii-font.c

# uncomment if you want it to automatically run.
RUN=1

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install
RUN = 1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
