# uncomment this when your hardware works and start
# writing the driver.
COMMON_SRC = i2c.c

# use last lab's code
# IMU_DIR := ../../7-imu-i2c/code/
# PROGS = $(IMU_DIR)/driver-gyro.c
# PROGS = $(IMU_DIR)/driver-accel.c

# really simple one: if this works, doesn't mean much.
# can be useful for debugging
PROGS = driver.c 

# when done, should use yours.
# pick either yours or ours.
COMMON_SRC += mpu-6050.c mpu-6050-gyro.c  mpu-6050-accel.c mpu-6050-test.c
# STAFF_OBJS += $(IMU_DIR)/staff-mpu-6050.o

# define this if you need to give the device for your pi
TTYUSB = 
BOOTLOADER = pi-install

# uncomment if you want it to automatically run.
RUN=1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
