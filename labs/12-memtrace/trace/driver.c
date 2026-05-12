#include "rpi.h"
#include "memtrace.h"

void notmain(void) {
    memtrace_t dat;

    // start tracing mapping
    memtrace_init(&dat);

    // allocate some memory
    uint32_t *v = kmalloc(sizeof *v);

    // start tracing
    memtrace_start();

    // be cheeky
    trace("putting 64 at %x\n", v);
    put32(v, 64);
    uint32_t got = get32(v);
    trace("got %d\n", got);

    // and stop tracing
    memtrace_stop();

    // print trace events 
    for (int i = 0; i < dat.n; i++) {
        memtrace_event *e = &dat.events[i];
        trace("event %x: pc=%x load_p=%d\n", e->addr, e ->pc, e->load_p);
    }
}
