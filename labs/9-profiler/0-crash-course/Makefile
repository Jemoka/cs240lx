PROGS += 0-nop1-example.c
PROGS += 1-many-fn-example.c

O := $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(O)/staff-kmalloc.o
STAFF_OBJS += $(O)/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-breakpoint.o
STAFF_OBJS += $(O)/staff-full-except.o

BOOTLOADER=my-install
RUN = 1
CAN_EMIT=1

OUR_START = single-step-start.S

# you can change opt level and the runs won't change.
# OPT_LEVEL = -O2

EXCLUDE ?= grep -v simple_boot

GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'
include $(CS140E_2026_PATH)/libpi/mk/Makefile.robust-v2
