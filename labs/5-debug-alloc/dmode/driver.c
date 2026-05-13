#include "rpi.h"
#include "gc.h"
#include "gc-malloc.h"

void notmain(void) {
    printk("heeeee\n");
    char *p = gcmalloc(4);
    *p = 1;
    printk("value: %d\n", *p);
    gcfree(p);
}

