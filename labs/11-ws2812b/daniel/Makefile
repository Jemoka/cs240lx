# make sure they all have fairly low error.
# PROGS := 2-write-01.c
# PROGS := 1-ndelay.c
PROGS := driver.c

# mess with the optimization level and see what happens!
# OPT_LEVEL = -O2 -fno-optimize-sibling-calls 
OPT_LEVEL = -Ofast -fno-builtin-memset
OPT_LEVEL = -O3

# used to get control reg 1.

BOOTLOADER=pi-install
RUN = 1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
