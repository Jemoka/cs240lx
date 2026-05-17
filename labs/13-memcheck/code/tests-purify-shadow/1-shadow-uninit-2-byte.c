// checks a single memory overflow.
#include "rpi.h"
#include "purify.h"

void notmain(void) {
    trace("should detect memory overflow at 1 byte past block end\n");

    purify_init();
    volatile uint16_t *p = purify_alloc(4);
    trace("about to read uninit [addr=%x]\n", p);
    *p;
    trace("should have caught uninit before now!\n");
}
