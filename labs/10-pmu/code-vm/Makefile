PROGS := 4-test-vm-cache-mgmt.c
PROGS := dcache-test.c

# map the staff_ names to regular ones.
COMMON_SRC += map-user-to-staff-fn.S

O = $(CS240LX_2026_PATH)/libpi/staff-objs/
STAFF_OBJS += $(O)/staff-kmalloc.o
STAFF_OBJS += $(O)/staff-pinned-vm.o
STAFF_OBJS += $(O)/staff-mmu-asm.o
STAFF_OBJS += $(O)/staff-mmu.o
STAFF_OBJS += $(O)/staff-full-except-asm.o  
STAFF_OBJS += $(O)/staff-full-except.o  
STAFF_OBJS += $(O)/staff-switchto-asm.o

RUN = 1

include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
