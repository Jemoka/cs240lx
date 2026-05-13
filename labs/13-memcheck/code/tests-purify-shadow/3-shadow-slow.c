// measures how long just a simple load takes with tracing.
#include "rpi.h"
#include "purify.h"
#include "cycle-count.h"

void notmain(void) {
    trace("should detect memory overflow at 1 byte past block end\n");
    // caches_enable();

    purify_init();
    // completely dumb loop to measure how long it takes
    // to trace and trap.
    enum { N = 8 };
    volatile char *p = purify_alloc(N);
    
    memset((void*)p, 0, N);

    let s = cycle_cnt_read();
    for(int i = 0; i < N; i++)
        p[i]++;
    let e = cycle_cnt_read();

    trace("DONE: %d array takes [%d] cycles\n", N, e-s);
}
