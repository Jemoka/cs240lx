#include "prelude.h"

static int l1, l2;

void notmain(void) {
    eraser_init();
    eraser_set_thread_id(1);

    // acquire lock <l>
    lock(&l1);

    // allocate some cheeky memory
    int *x = gcmalloc(4);
    put32(x,0x12345678);   // holding lock <l>: this is fine.
    int ret = get32(x);    // holding lock <l>: this is fine.
    unlock(&l1);

    // we "context switch"
    eraser_set_thread_id(2);

    // goes to shared mod, but has a consistent lock so is ok.
    // that is, its unlocked, and a new thread got the lock, so nice
    lock(&l2);
    put32(x,0x12345678);   // should be fine.
    put32(x,0x12345678);   // should be fine.
    put32(x,0x12345678);   // should be fine.
    put32(x,0x12345678);   // should be fine.
    put32(x,0x12345678);   // should be fine.
    put32(x,0x12345678);   // should be fine.
    unlock(&l2);

    trace("SUCCESS\n");
}

