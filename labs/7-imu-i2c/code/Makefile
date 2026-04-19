PROGS = driver-gyro.c
PROGS += driver-accel.c

# when you build yours swap this.
# COMMON_SRC = mpu-6050.c
STAFF_OBJS += staff-mpu-6050.o
L := $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(L)/i2c.o

# set to 0 if you don't want it to automatically run.
RUN=0

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = my-install

# for division
include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
