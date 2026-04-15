PROGS := test-fn.c

CFLAGS += -finstrument-functions
CFLAGS += -finstrument-functions-exclude-file-list=libpi

# COMMON_SRC += no-instrument.c

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER := my-install

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3

no-instrument.o: no-instrument.c
	$(CC) -c no-instructment.c
