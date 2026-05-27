PROGS := driver.c
# PROGS := 1-draw-commands.c

COMMON_SRC += display.c song.S
# COMMON_SRC += standard-ascii-font.c

# uncomment if you want it to automatically run.
# RUN=1

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install
RUN = 1

MEMMAP=./memmap.ld
# PREBUILT_OBJS = i2c.o

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
# include $(CS340LX_2025_PATH)/libpi/mk/Makefile.robust
