// check that a pointer to the middle of a block keeps it alive until cleared.
#include "rpi.h"
#include "ckalloc.h"

static char *middle_p;

static void scrub_stack(void) {
    volatile uint32_t x[32];

    for(unsigned i = 0; i < sizeof x / sizeof x[0]; i++)
        x[i] = i + 7;
    gcc_mb();
}

static char *alloc_middle_root(void) {
    char *p = ckalloc(8);
    memset(p, 0x55, 8);
    return p + 1;
}

void notmain(void) {
    printk("GC test9: middle pointers should keep blocks alive until cleared.\n");

    middle_p = alloc_middle_root();
    scrub_stack();

    trace("with only a middle pointer: should have no definite leaks\n");
    if(ck_find_leaks(0))
        panic("should have no definite leaks!\n");

    put32(&middle_p, 0);
    scrub_stack();

    trace("after clearing the middle pointer: should free the block\n");
    unsigned n = ck_gc();
    if(n != 8)
        panic("gc should free 8 bytes: got %d\n", n);

    trace("second gc: should free nothing\n");
    n = ck_gc();
    if(n)
        panic("second gc should free 0 bytes: got %d\n", n);

    check_no_leak();
}
