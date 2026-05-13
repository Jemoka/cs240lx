#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"
#include "sbrk-trap.h"

static int handler(void *counts, memtrace_event *e) {
    unsigned *n = counts;
    *n += 1;

    trace("event %x: pc=%x load_p=%d\n", e->addr, e ->pc, e->load_p);

    return 0;
}

void notmain(void) {
    // initialize sbrk (such that kmalloc will be inside the trapped region)
    sbrk_init();

    // keep a count
    unsigned n = 0;

    // allocate some memory in the faulting domain
    uint32_t *v = kmalloc(sizeof *v);

    // intialize
    memtrace_init(&n, handler, 0, dom_trap);

    // start tracing
    memtrace_start();

    // be cheeky
    put32(v, 0xfacefeed);
    uint32_t got = get32(v);

    // finish being cheeky
    memtrace_stop();

    trace("got %x\n", got);
    demand(got == 0xfacefeed, "no");


}
