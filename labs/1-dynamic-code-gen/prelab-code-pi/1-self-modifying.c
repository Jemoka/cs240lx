// simple example of self-modifying code.
#include "rpi.h"

/*
   from the list file:
    00008038 <ret_10>:
        8038:   e3a0000a    mov r0, #10
        803c:   e12fff1e    bx  lr
 */
int ret_10(void) { return 10; }

/* 
   from the list file:
    00008040 <ret_11>:
        8040:   e3a0000b    mov r0, #11
        8044:   e12fff1e    bx  lr
 */
int ret_11(void) { return 11; }


void notmain() { 
    // Q: if we enable the icache, what is going on?
    //enable_cache();

    // generate the code to return 11
    //
    // Q: why might expect this needs to be volatile?
    // [doesn't seem to w/ our compiler]
	unsigned code[3];

    // from above:
    //  00008040 <ret_11>:
    //      8040:   e3a0000b    mov r0, #11
    //      8044:   e12fff1e    bx  lr
    code[0] = 0xe3a0000b;       // mov r0, #11
    code[1] = 0xe12fff1e;       // bx lr

    // cast array address to function pointer.
	int (*fp)(void) = (int (*)(void))code;

    // call it.
    unsigned x = fp();
	printk("fp() = %d [should be 11]\n", x);
    assert(x == 11);

    // change the code to return 10
    printk("about to make code return 10\n");
    asm volatile(".align 5");
    code[0] = 0xe3a0000a; // mov r0, #10

    // Q: experiment w/ deleting these: what is going on?
    // Q: how many can we delete? why?
    asm volatile ("nop"); // 0
    asm volatile ("nop"); // 1
    asm volatile ("nop"); // 2
    asm volatile ("nop"); // 3
    asm volatile ("nop"); // 4
    asm volatile ("nop"); // 5
    asm volatile ("nop"); // 6
    asm volatile ("nop"); // 7

    x = fp();
	printk("fp() = %d [should be 10]\n", x);
    assert(x == 10);

    enum { N = 255 };
    printk("about to try [0,%d)\n", N);
    for(unsigned u = 0; u < N; u++) {
        // clear and set the bits for immediate
        //
        // Q: how do you set them to return a negative value?
        code[0] = (code[0] & ~0xff) | u;

        // Q: if delete this?  how different from nops?
        asm volatile(".align 5");
        assert(fp() == u);
    }
    printk("passed %d tests!\n", N);
}
