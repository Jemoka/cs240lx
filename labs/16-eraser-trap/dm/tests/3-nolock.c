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

    // bad lock case
    trace("should get an error b/c we don't have a lock\n");
    put32(x,0x12345678);   // bug

    trace("SUCCESS\n");
}

