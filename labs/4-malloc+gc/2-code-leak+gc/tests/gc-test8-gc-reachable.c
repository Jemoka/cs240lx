// check that a live root keeps the heap objects it points to alive.
#include "rpi.h"
#include "ckalloc.h"

struct root {
    char *child;
    uint32_t tag;
};

static struct root *root_p;

static void scrub_stack(void) {
    volatile uint32_t x[32];

    for(unsigned i = 0; i < sizeof x / sizeof x[0]; i++)
        x[i] = i + 1;
    gcc_mb();
}

static void alloc_graph(void) {
    root_p = ckalloc(sizeof *root_p);
    memset(root_p, 0, sizeof *root_p);
    root_p->tag = 0xdeadbeef;
    root_p->child = ckalloc(4);
    memset(root_p->child, 0x33, 4);
}

void notmain(void) {
    printk("GC test8: reachable heap objects should stay live until the root is cleared.\n");

    alloc_graph();

    trace("with a live root: should not leak\n");
    check_no_leak();

    put32(&root_p, 0);
    scrub_stack();

    trace("after clearing the root: should free the root and child\n");
    unsigned n = ck_gc();
    if(n != sizeof *root_p + 4)
        panic("gc should free 12 bytes: got %d\n", n);

    trace("second gc: should free nothing\n");
    n = ck_gc();
    if(n)
        panic("second gc should free 0 bytes: got %d\n", n);

    check_no_leak();
}
