#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

void notmain(void) {
    char *p = gcmalloc(4);
    memset(p, 0, 4);
    p[4] = 1;
    gcfree(p);

    panic("UNREACHABALE\n");
}

