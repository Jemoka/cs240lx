# PROGS += 0-test-malloc.c
# PROGS += 1-test-gcmalloc.c
# PROGS += driver.c
PROGS += 2-test-simple-gc.c
# PROGS += 3-leak.c

COMMON_SRC = pi-sbrk.c
COMMON_SRC += kr-malloc.c 
COMMON_SRC += gc-malloc.c 
COMMON_SRC += gc.c
COMMON_SRC += gc-asm.S

# STAFF_OBJS = staff-kr-malloc.o

RUN=1

EXCLUDE ?= grep -v simple_boot
include $(CS240LX_2026_PATH)/libpi/mk/Makefile.robust-v3
