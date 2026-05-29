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

    // immutable borrows are always ok
    get32(x);    // not an error 
    get32(x);    // not an error 

    eraser_set_thread_id(2);
    get32(x);    // still ok, even across thread bondauries
    get32(x);    // still ok, even across thread bondauries
    get32(x);    // still ok, even across thread bondauries

    // still ok
    eraser_set_thread_id(3);
    get32(x);    // still ok, even across thread bondauries
    get32(x);    // still ok, even across thread bondauries
    get32(x);    // still ok, even across thread bondauries

    // everlasting ok
    eraser_set_thread_id(1);
    get32(x);    // not an error 
    get32(x);    // not an error 

    trace("SUCCESS\n");
}

