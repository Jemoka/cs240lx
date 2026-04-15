#include "rpi.h"
#include "kr-malloc.h"

void notmain(void) {
    uint32_t *test = (uint32_t *) krmalloc(sizeof(uint32_t)*4);
    test[0] = 0x12345678;
    test[1] = 0xdeadbeef;
    test[2] = 0xabcdef01;
    test[3] = 0xfeedbabe;
    
    for (int i = 0; i < 4; i++) {
        printk("%x\n", test[i]);
    }

    krfree(test);

    test = (uint32_t *) krmalloc(sizeof(uint32_t)*4);
    demand(test[0] == 0x12345678, "krmalloc did not return the same block that was just freed?");

    for (int i = 0; i < 4; i++) {
        printk("%x\n", test[i]);
    }

    uint32_t *next = (uint32_t *) krmalloc(sizeof(uint32_t)*4);
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

    krfree(test);    
    krfree(next);

    test = (uint32_t *)krmalloc(sizeof(uint32_t) * 4);
    demand(test[0] == 0x12345678, "krmalloc did not return the same block that was just freed?");
}  

