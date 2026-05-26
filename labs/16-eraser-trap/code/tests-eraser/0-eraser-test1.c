// engler: simple lock and unlock with one piece of shared state: 
//   - should have no errors.
#include "eraser.h"

// has the lock/unlock etc implementation.
#include "fake-thread.h"

static int l;

void notmain() {
    trace("we expect no error\n");
    eraser_init();
    eraser_verbose_set(1);

    // set the current thread id.  normally thread package
    // would do this but we strip things down to make debug
    // easier.
    eraser_set_thread_id(1);

    // acquire lock <l>
    lock(&l);

    // NOTE: kmalloc zeroes out the the memory (using stores)
    // so you'll have to make an eraser_alloc that disables
    // enables trapping if you want to allocate outside of lock.
    int *x = kmalloc(4);

    put32(x,0x12345678);   // holding lock <l>: this is fine.
    int ret = get32(x);    // holding lock <l>: this is fine.
    unlock(&l);

    // turn off verbose so we don't get flooded.
    eraser_verbose_set(0);
    for(int i = 0; i < 10; i++) {
        // multiple accesses with same lock <l> are legal.
        trace("going to increment x=<%p> again with lock\n", x);
        lock(&l);
        *x += 1;
        unlock(&l);
    }
    trace_clean_exit("success!!\n");
}
