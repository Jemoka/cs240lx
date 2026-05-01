#include "rpi.h"
#include "ss-pixie.h"

extern void cheeky(void); 

void notmain(void) {
    // caches_enable();     // Q: what happens if you enable cache?

    pixie_verbose(0);
    pixie_start();
    cheeky();
    unsigned n = pixie_stop();
    pixie_dump(0);
    trace("done: traced [%d] instructions\n", n);
}
