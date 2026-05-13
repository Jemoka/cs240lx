// checks a single memory overflow.
#include "rpi.h"
#include "purify.h"

void notmain(void) {
    trace("uninitialized load of byte-1 of a 4-byte load\n");

    purify_init();
    volatile uint32_t *p = purify_alloc(4);
    volatile uint8_t *q = (void*)p;

    trace("about to read uninit [addr=%x]\n", p);
    // initialize the low 3 bytes
    q[0] = 0;
    q[3] = 0;
    q[2] = 0;
    *p;         // should catch this.
    trace("should have caught uninit before now!\n");
}
