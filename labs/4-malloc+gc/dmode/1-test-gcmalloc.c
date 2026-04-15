#include "rpi.h"
#include "gc-malloc.h"

void notmain(void) {
    uint32_t *test = (uint32_t *) gcmalloc(sizeof(uint32_t)*4);
    test[0] = 0x12345678;
    test[1] = 0xdeadbeef;
    test[2] = 0xabcdef01;
    test[3] = 0xfeedbabe;
    
    for (int i = 0; i < 4; i++) {
        printk("%x\n", test[i]);
    }

    gcfree(test);

    test = (uint32_t *) gcmalloc(sizeof(uint32_t)*4);
    printk("%x\n", test[0]);
    demand(test[0] == 0x12345678, "gcmalloc did not return the same block that was just freed?");

    for (int i = 0; i < 4; i++) {
        printk("%x\n", test[i]);
    }

    uint32_t *next = (uint32_t *) gcmalloc(sizeof(uint32_t)*4);
    next[0] = 0x11111111;
    next[1] = 0x22222222;    
    next[2] = 0x33333333;
    next[3] = 0x44444444;

    for (int i = 0; i < 4; i++) {
        printk("%x\n", test[i]);
    }

    for (int i = 0; i < 4; i++) {
        printk("%x\n", next[i]);
    }

    gcfree(test);    
    gcfree(next);

    test = (uint32_t *)gcmalloc(sizeof(uint32_t) * 4);
    demand(test[0] == 0x12345678, "gcmalloc did not return the same block that was just freed?");
}  

