#include "rpi.h"
#include "gc-malloc.h"
#include "gc.h"

// memory leak test

void leak(void) {
    uint32_t *test = (uint32_t *) gcmalloc(sizeof(uint32_t)*4);
    test[0] = 0x12345678;
    test[1] = 0xdeadbeef;
    test[2] = 0xabcdef01;
    test[3] = 0xfeedbabe;
    printk("%x\n", test);
    // we don't free weee
}  

void notmain(void) {
    leak();
    gc();
    uint32_t *test = (uint32_t *) gcmalloc(sizeof(uint32_t)*4);
    printk("%x\n", test);
}
