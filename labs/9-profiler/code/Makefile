# very simple tests. start here.

# PROGS := tests/calibration.c
# PROGS += tests/0-prof-test.c
# PROGS += tests/0-reg-test.c

# when you want to start profiling
PROGS := tests/1-prof-test.c

COMMON_SRC := ss-pixie.c
COMMON_SRC += ss-pixie-asm.S

# use a different start so we have routines in known
# locations.
OUR_START = staff-start.S

# you can swap in your 140e
O := $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(O)/staff-breakpoint.o

BOOTLOADER=pi-install
RUN = 1
# CAN_EMIT=1


EXCLUDE ?= grep -v simple_boot
include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
