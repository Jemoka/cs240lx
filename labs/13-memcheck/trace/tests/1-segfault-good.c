#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"
#include "sbrk-trap.h"
#include "gc-malloc.h"

static int segfaulter(void *counts, memtrace_event *e) {
    unsigned *n = counts;
    *n += 1;

    int error = gcgood((char *) e->addr);

    // 0 is good!
    if (!error) { return 0; }

    // 1 is "we don't know about this pointer"
    if (error == 1) {
        panic("segv: use of non-managed heap memory %x\n", e->addr);
    }

    // 2 is weird, so we can find it
    int dist = gcdist((char *) e->addr);
    panic("segv: managed memory; your pointer %x is %d away from a good one?\n", e->addr, dist);

    return 0;
}

void notmain(void) {
    // initialize sbrk (such that kmalloc will be inside the trapped region)
    sbrk_init();

    // keep a count
    unsigned n = 0;

    // allocate some memory in the faulting domain
    uint32_t *v = gcmalloc(sizeof *v);

    // intialize
    memtrace_init(&n, segfaulter, 0, dom_trap);

    // start tracing
    memtrace_start();

    // be rather not very cheeky
    put32(v, 0xfacefeed);
    uint32_t got = get32(v);

    // finish tracing
    memtrace_stop();

    trace("got %x\n", got);
    demand(got == 0xfacefeed, "no");


}
