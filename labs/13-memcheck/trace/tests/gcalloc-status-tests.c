#include "rpi.h"
#include "memtrace.h"
#include "memmap-default.h"
#include "sbrk-trap.h"
#include "gc-malloc.h"

void notmain(void) {
    sbrk_init();
    char *v = gcmalloc((sizeof *v)*2);

    demand(gcdist(v)==0, "good pointer is not good?");
    demand(gcdist(v+1)==0, "good pointer is not good?");
    demand(gcdist(v-1)==-1, "front redzone check failed");
    demand(gcdist(v+2)==1, "back redzone check failed");

    demand(gcgood((char *) 0xdeadbeef) == 1, "unmanaged block is not managed?");
    demand(gcgood(v) == 0, "good block is not good?");
    gcfree(v);
    demand(gcgood(v) == 2, "free block is not free?");

    printk("yay!\n");
}
