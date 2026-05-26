// engler: do one safe store and one unsafe load.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {
    eraser_init();
    // if you want to see transitions, change to 1:
    eraser_verbose_set(0);

    eraser_set_thread_id(1);

    lock(&l);
    int *x = kmalloc(4);
    put32(x,0x12345678);   // should be fine: we hold <l>
    unlock(&l);

    trace("--------------------------------------------------\n");
    trace("expect a store error at pc=%p, addr=%p:\n", get32, x);
    put32(x,0x12345678);    // error: we don't have a lock.

    panic("should have caught unprotected access error\n");

}
