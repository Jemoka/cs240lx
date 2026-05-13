#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

void notmain(void) {
    char *p = gcmalloc(4);
    memset(p, 0, 4);
    gcfree(p);

    demand(!check_heap(), "heap check failed, hallucinated use-after-free detected\n");
}

