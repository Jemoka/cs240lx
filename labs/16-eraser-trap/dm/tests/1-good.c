#include "prelude.h"

static int l;

void notmain(void) {
    eraser_init();
    eraser_set_thread_id(1);

    // acquire lock <l>
    lock(&l);

    // allocate some cheeky memory
    int *x = gcmalloc(4);
    put32(x,0x12345678);   // holding lock <l>: this is fine.
    int ret = get32(x);    // holding lock <l>: this is fine.
    unlock(&l);

    // let's keep going and try to access the thing
    for(int i = 0; i < 10; i++) {
        // multiple accesses with same lock <l> are legal.
        trace("going to increment x=<%p> again with lock\n", x);
        lock(&l);
        *x += 1;
        unlock(&l);
    }

    trace("SUCCESS\n");
}

