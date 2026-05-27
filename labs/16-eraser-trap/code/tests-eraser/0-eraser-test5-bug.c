// engler: do several thread switches: should have no error.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {

    trace("inconsistent lock, multiple threads should fail\n");
    eraser_init();
    int *x = kmalloc(4);

    eraser_set_thread_id(1);
    lock(&l);
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    put32(x,0x12345678);   // should be fine: holds <l>
    unlock(&l);

    eraser_set_thread_id(2);
    lock(&l);
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    put32(x,2);   // should be fine: holds <l>
    unlock(&l);

    eraser_set_thread_id(3);
    trace("-----------------------------------------------\n");
    trace("should be an error next access (no lock held):\n");
    put32(x,3);   // should be an error.

    panic("should have failed\n");
}
