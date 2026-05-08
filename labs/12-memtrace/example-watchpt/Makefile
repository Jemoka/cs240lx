PROGS := watchpoint-example.c

O = $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(O)/staff-kmalloc.o
STAFF_OBJS += $(O)/staff-watchpoint.o
STAFF_OBJS += $(O)/staff-full-except.o
STAFF_OBJS += $(O)/staff-full-except-asm.o
STAFF_OBJS += $(O)/staff-switchto-asm.o



OPT_LEVEL = -O2

BOOTLOADER=my-install
RUN = 1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
