#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

void notmain(void) {
    char *p = gcmalloc(4);
    memset(p, 0, 4);
    gcfree(p);

    p[0]=1;

    demand(check_heap() == 1, "heap check detected more errors than there were.\n");
}

