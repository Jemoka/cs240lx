#include "gc-malloc.h"

void notmain(void) {
    char *p = gcmalloc(4);
    *p = 1;
}

