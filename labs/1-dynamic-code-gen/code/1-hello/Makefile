PROGS = hello.c

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# so the guard routines don't get re-ordered.
CFLAGS += -fno-toplevel-reorder

# uncomment if you want it to automatically run.
RUN=0

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
