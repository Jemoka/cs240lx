// a bit fancier: allocate a chunk of trapping memory, iterate over
// it taking faults, verify that the answer is correct.  takes
// around 30k faults with the current values.  just change <N>
// or <K> to bump this number up!
//
// great extension: speed this up alot!
#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"

// need to specify the size: 1,2,4,8,... --- gcc is allowed to merge
// adjacent object writes, but i don't think will ever be able to see.
//
// probably better to pass in the fault.  but then you have to eat the
// load misses.

// 0 = quiet.
enum { quiet_p = 0 };

static int trace_handler(void *data, fault_ctx_t *f) {
    unsigned *n = data;
    *n += 1;

    if(!quiet_p)
        output("\t%d: memtrace: pc=%x, addr=%x, nbytes=%d, load=%d\n",
                    *n, 
                    f->pc,
                    f->addr, 
                    f->nbytes, 
                    f->load_p);

    return MEMTRACE_OK;
}



void notmain(void) {
    int n_faults = 0;

    memtrace_init(&n_faults, trace_handler, 0, dom_trap);
    memtrace_yap_off();

    volatile struct foo {
        uint32_t x[4];
    } *f, fv;

    volatile struct bar {
        uint32_t x[3];
    } *b, bv;


    f = kmalloc(sizeof *f);
    b = kmalloc(sizeof *b);

    memtrace_trap_enable();

    output("about to write %d bytes\n", sizeof fv);
    // so it's easy to find.
    // asm volatile("nop; nop; nop; nop;");
    *f = fv;
    output("about to write %d bytes\n", sizeof bv);
    *b = bv;
    memtrace_trap_disable();

    trace("SUCCESS: total faults = %d\n", n_faults);
}
