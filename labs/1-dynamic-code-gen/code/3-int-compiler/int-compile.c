#include "rpi.h"
#include "bit-support.h"

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#include "cycle-util.h"

typedef void (*int_fp)(void);

static volatile unsigned cnt = 0;

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

// fake little "interrupt" handlers: useful just for measurement.
void int_0() { cnt++; }
void int_1() { cnt++; }
void int_2() { cnt++; }
void int_3() { cnt++; }
void int_4() { cnt++; }
void int_5() { cnt++; }
void int_6() { cnt++; }
void int_7() { cnt++; }

void generic_call_int(int_fp *intv, unsigned n) { 
    for(unsigned i = 0; i < n; i++)
        intv[i]();
}

// you will generate this dynamically.
void specialized_call_int(int_fp *intv, unsigned j) {
    static char lr = 14;
    static uint32_t code[128];
    int n = 0;
    code[n++] = armv6_push(lr);
    uint32_t src;

    for(unsigned i = 0; i < j; i++) {
        src = (uint32_t) &code[n];
        code[n++] = armv6_bl(src, (uint32_t) intv[i]);
    }
    /* return code; */
    code[n++] = armv6_pop(lr);
    code[n++] = armv6_bx(lr);

    void (*fp)(void) = (typeof(fp))code;
    TIME_CYC_PRINT10("cost of specialized int calling", fp() );
}

void notmain(void) {
    int_fp intv[] = {
        int_0,
        int_1,
        int_2,
        int_3,
        int_4,
        int_5,
        int_6,
        int_7
    };

    cycle_cnt_init();

    unsigned n = NELEM(intv);

    // try with and without cache: but if you modify the routines to do 
    // jump-threadig, must either:
    //  1. generate code when cache is off.
    //  2. invalidate cache before use.
    // enable_cache();

    cnt = 0;
    TIME_CYC_PRINT10("cost of generic-int calling",  generic_call_int(intv,n));
    demand(cnt == n*10, "cnt=%d, expected=%d\n", cnt, n*10);

    // rewrite to generate specialized caller dynamically.
    cnt = 0;
    specialized_call_int(intv, n);
    demand(cnt == n*10, "cnt=%d, expected=%d\n", cnt, n*10);

    clean_reboot();
}
