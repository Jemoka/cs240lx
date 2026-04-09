// check that globals are treated as roots until they are cleared.
#include "rpi.h"
#include "ckalloc.h"

static char *global_p;

static void scrub_stack(void) {
    volatile uint32_t x[32];

    for(unsigned i = 0; i < sizeof x / sizeof x[0]; i++)
        x[i] = i;
    gcc_mb();
}

static void alloc_global_root(void) {
    global_p = ckalloc(4);
    memset(global_p, 0x11, 4);
}

void notmain(void) {
    printk("GC test7: globals are gc roots until cleared.\n");

    alloc_global_root();

    trace("with a global root: should not leak\n");
    check_no_leak();

    put32(&global_p, 0);
    scrub_stack();

    trace("after clearing the global root: should free the block\n");
    unsigned n = ck_gc();
    if(n != 4)
        panic("gc should free 4 bytes: got %d\n", n);

    trace("second gc: should free nothing\n");
    n = ck_gc();
    if(n)
        panic("second gc should free 0 bytes: got %d\n", n);

    check_no_leak();
}
