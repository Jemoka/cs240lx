// dynamically generate code to call a routine.  we
// do this in two steps to make it easier to debug.
//  - first, generate a call to a predefined functions 
//    that take no arguments (hello_before and hello_after).
//  - when that works, generate a call to a routine that takes
//    a 32-bit argument.  the general way that arm handles this:
//       - write the 32-bit value in the code array
//         *after* all the code you generate
//       - emit an ldr instruction using the pc to load
//         it.
#include "rpi.h"
#include "rpi-interrupts.h"
#include "bit-support.h"

// we use this to catch if you jump off by one
static void guard1(void) { asm volatile ("bkpt"); }

static void hello1(void) { 
    printk("hello world 1\n");
}

// we use this to catch if you jump off by one
static void guard2(void) { asm volatile ("bkpt"); }

void hello2(void) { 
    printk("hello world 2\n");
}

// we use this to catch if you jump off by one
static void guard3(void) { asm volatile ("bkpt"); }

// the routines to implement.
static inline uint32_t armv6_push(int reg) {
    assert(reg<16);

    // (str fp, [sp, #-4]!)
    // 0 1 0 1 0 0 1 0 Rn Rd offset_12
    //          16-19 _|  |_ 12-15  |_ 0-11
    unsigned base = 0b0101001000000000000000000000;

    base = bits_set(base, 16, 19, 13); // sp is Rn
    base = bits_set(base, 12, 15, reg); // reg is Rd
    base = bits_set(base, 0, 11, 4); // offset is 4

    return base;
}
static inline uint32_t armv6_pop(int reg) {
    assert(reg<16);             /*  */

    // (ldr fp, [sp], #4)
    // 0 1 0 0 1 0 0 1 Rn Rd 0 0 0 0 0 0 0 0 Rm
    //          16-19 _|  |_ 12-15           |_ 0-3

    unsigned base = 0b0100100100000000000000000000;

    base = bits_set(base, 16, 19, 13); // sp is Rn
    base = bits_set(base, 12, 15, reg); // reg is Rd
    base = bits_set(base, 0, 3, 4); // offset is 4
    
    return base;
}

// pc = where the instruction will be put.  this is 
// needed so that you can compute the offset from <pc>
// to <addr> which is what gets put in <bl>
static inline uint32_t armv6_bl(uint32_t bl_pc, uint32_t target) {
    // this is just a standard BL instruction
    unsigned base = 0b1011 << 24;

    // and now compute the offset from <bl_pc> to <target>
    int signed_imm_24 = (int) target - (int) bl_pc - 8;

    // set the first 23 bits to the offset address
    base = bits_set(base, 0, 23, (signed_imm_24 >> 2) & 0x00FFFFFF);

    return base;    
}
static inline uint32_t armv6_bx(uint32_t reg) {
    assert(reg<16);
    int base = 0b0001001011111111111100010000;
    base = bits_set(base, 0, 3, reg);
    return base;
}

/* return the machine code to: ldr <dst>, [<src>+#<off>]\n */
static inline uint32_t 
armv6_ldr(uint32_t dst_reg, uint32_t src_reg, uint32_t off) {
    assert(dst_reg<16);
    assert(src_reg<16);

    // addressing mode is [<Rn>, #+/-<offset_12>]
    /* 0 1 0 1 1 0 1 1 Rn Rd offset_12 */
    //           16-19 _|  |_ 12-15  |_ 0-11
    unsigned base = 0b0101101100000000000000000000;
    base = bits_set(base, 16, 19, src_reg);
    base = bits_set(base, 12, 15, dst_reg);
    base = bits_set(base, 0, 11, off);
    return base;    
}

// generate a dynamic call to hello() 
void jit_hello(void *fn, void * arg) {
    static uint32_t code[8];
    

    // a few of the registers
    enum {
        lr = 14,
        pc = 15,
        sp = 13,
        r0 = 0,
    };

    uint32_t addr =(uint32_t)fn;
    uint32_t n = 0;

    // generate a trampoline to call <fn>.
    //   1. we need to save and restore <lr> since the 
    //      call (bl) will trash it.
    //   2. need to make sure you sign extend <addr> in bl
    //      correctly so that it works with a negative offset!
    code[n++] = armv6_push(lr);

    // to see what value the pc register has when you read it
    // you can look at <prelab-code-pi/4-derive-pc-reg.c>
    // or also read the manual :)
    if(arg) {
        code[n++] = armv6_ldr(r0, pc, 8); // load the argument into r0
        /* printk("the PC register will be %x when the ldr executes\n", (uint32_t)&code[n] + 8+4); */
    }

    uint32_t src = (uint32_t)&code[n];
    code[n++] = armv6_bl(src, addr);
    code[n++] = armv6_pop(lr);
    code[n++] = armv6_bx(lr);
    code[n++] = (uint32_t) arg;
    printk("the argument is placed in code[5]=%x\n", &code[n-1]);

    printk("emitted code at %x to call routine (%x):\n", code, addr);
    for(int i = 0; i < 4; i++) 
        printk("code[%d]=0x%x\n", i, code[i]);

    void (*fp)(void) = (typeof(fp))code;
    printk("about to call: %x\n", fp);
    printk("--------------------------------------\n");
    fp();
    printk("--------------------------------------\n");
}

void notmain(void) {
    // so we can catch some exceptions.
    interrupt_init();

    // step 1: generate calls that take no arguments.
    jit_hello(hello1, 0);
    jit_hello(hello2, 0);
    // step 2: generate calls that take a single argument
    jit_hello(printk, "hello world\n");
    jit_hello(printk, "hello world2\n");
}
