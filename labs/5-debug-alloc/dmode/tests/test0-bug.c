#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

void notmain(void) {
    char *p = gcmalloc(4);
    gcfree(p);
    *p = 1;

    demand(check_heap(), "heap check failed, use-after-free not detected\n");
}

