// a bit fancier: allocate a chunk of trapping memory, iterate over
// it taking faults, verify that the answer is correct.  takes
// around 30k faults with the current values.  just change <N>
// or <K> to bump this number up!
//
// great extension: speed this up alot!
#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"

#include "sbrk-trap.h"

// need to specify the size: 1,2,4,8,... --- gcc is allowed to merge
// adjacent object writes, but i don't think will ever be able to see.
//
// probably better to pass in the fault.  but then you have to eat the
// load misses.

// 0 = quiet.

static int trace_handler(void *data, memtrace_event *e) {
    unsigned *n = data;
    *n += 1;
    return 0;
}



void notmain(void) {
    sbrk_init();
    int n_faults = 0;

    memtrace_init(&n_faults, trace_handler, 0, dom_trap);

    volatile struct foo {
        uint32_t x[4];
    } *f, fv;

    volatile struct bar {
        uint32_t x[3];
    } *b, bv;


    f = kmalloc(sizeof *f);
    b = kmalloc(sizeof *b);

    memtrace_start();

    output("about to write %d bytes\n", sizeof fv);
    // so it's easy to find.
    // asm volatile("nop; nop; nop; nop;");
    *f = fv;
    output("about to write %d bytes\n", sizeof bv);
    *b = bv;
    memtrace_stop();

    trace("SUCCESS: total faults = %d\n", n_faults);
}
