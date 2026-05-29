PROGS := driver.c

INC += -I./includes
COMMON_SRC += eraser.c
COMMON_SRC += memtrace.c
COMMON_SRC += memmap-default.c
COMMON_SRC += sbrk-trap.c
COMMON_SRC += kr-malloc.c
COMMON_SRC += gc-malloc.c

O = $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(O)/staff-kmalloc.o
STAFF_OBJS += $(O)/interrupts-vec-asm.o
STAFF_OBJS += $(O)/staff-pinned-vm.o
STAFF_OBJS += $(O)/staff-mmu-asm.o
STAFF_OBJS += $(O)/staff-mmu.o
STAFF_OBJS += $(O)/staff-watchpoint.o
STAFF_OBJS += $(O)/staff-armv6-except.o
STAFF_OBJS += $(O)/staff-breakpoint.o
STAFF_OBJS += $(O)/staff-switchto-asm.o
STAFF_OBJS += $(O)/staff-full-except.o
STAFF_OBJS += $(O)/staff-full-except-asm.o
STAFF_OBJS += $(O)/map-user-to-staff-fn.o

# can try different opt levels and see that its consistent.
# OPT_LEVEL = -O2

INC += -I./includes
BOOTLOADER = pi-install
RUN = 1
include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
