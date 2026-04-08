#ifndef __derive__h__
#define __derive__h__

#include "rpi.h"
#include "repeat.h"


// helper to print stuff
#define BITS_STR(_x) ({                            \
    uint32_t b = _x;                                \
    static char buf[33]={0};                        \
    for(uint32_t i = 32; i > 0; i--)                \
        buf[32-i] = "01"[((b) >> (i-1)) & 1];            \
    buf;                                            \
})

#define SENTINAL ".word 0x12345678;"

typedef struct {
    unsigned start;
    unsigned end;
    unsigned n;
} bitregion;

typedef struct {
    uint32_t *start;
    uint32_t *end;
    uint32_t n;
} inst;

typedef struct {
    uint32_t template;
    uint32_t changed;
    uint32_t unchanged;
} derived;

// find where the sentinal is in the code, so we can patch it with the derived code
static uint32_t *find_sentinal(uint32_t *p, unsigned max_len) {
    for(int i = 0; i < max_len; i++)
        if(p[i] == 0x12345678)
            return &p[i];
    panic("did not find sentinal after %d instructions\n", max_len);
    return 0;
}

// generate code that iterates through each register for us to find the diff
#define REPEAT(F,N)                              \
asm volatile ( \
    SENTINAL \
    REPEAT_INNER(F,N)                           \
    SENTINAL \
);

#define GEN_DERIVE_PRIM(name, F,N)              \
    void derive_inner_##name(void) {           \
        REPEAT(F,N)                             \
    }

// function that returns the start and end of the generated codeblock
#define MAKE_DERIVE_HELPER(name, F,N)                                    \
    GEN_DERIVE_PRIM(name, F,N)                                           \
      inst derive_helper_##name(void) {                                                   \
          uint32_t *start = find_sentinal((void *)derive_inner_##name, 128) + 1; \
          uint32_t *end = find_sentinal(start, 128);\
          unsigned n = end - start;\
            return (inst){.start = start, .end = end, .n = n}; \
        }

// actual derivation that ORs things together
static derived derivate(inst (*derive_prim)(void)) {
    inst ins = derive_prim();
    uint32_t *inst = ins.start;

    uint32_t always_0 = ~0;
    uint32_t always_1 = ~0;
    for(int i = 0; i < ins.n; i++) {
        always_0 &= ~inst[i];
        always_1 &= inst[i];
    }

    uint32_t changed = ~(always_0 | always_1);
    uint32_t unchanged = (always_0 | always_1);

    return (derived){.template = *inst, .changed = changed, .unchanged = unchanged};
};

// finally, the thing to call from notmain
#define DERIVE(name, F, N)                        \
    MAKE_DERIVE_HELPER(name, F, N);                 \
      static inline derived derive_##name(void) { \
          return derivate(derive_helper_##name); \
      }

#endif

// figure out where a continuous region of 1s starts and ends in the bitmask,
// and return it as a region struct
static inline bitregion bitmask_to_region(uint32_t bitmask) {
    bitregion r = {0};
    unsigned state = 0; // start
    for(unsigned i = 0; i < 32; i++) {
        if (!state && ((bitmask >> i) & 0b1)) {
            state = 1;
            r.start = i;
        } else if (state && !((bitmask >> i) & 0b1)) {
            state = 0;
            r.end = i;
            goto skip;
        }
    }
    r.end = 31;

 skip:
    r.n = (r.end-r.start);
    return r;
}

static inline uint32_t setmask(uint32_t base, uint32_t bitmask, uint32_t value) {
    bitregion r = bitmask_to_region(bitmask);
    uint32_t mask = value << r.start;
    return (base & ~bitmask) | (mask & bitmask);
}

// quick derive: define a function with that one line, and then
// get the instructions
#define GETOPCODE(name, ASM)                        \
    void quick_derive_inner_##name(void) { \
        asm volatile (ASM); \
    } \
    static inline uint32_t get_##name(void) {                \
        return ((uint32_t *)  quick_derive_inner_##name)[0]; \
    } \



