// similar to 4-dma-inc, but now we're adding together two numbers
//
// Note that this file has three parts in it, so it's normal for it to panic at first

#include "rpi.h"
#include "dma-impl.h"
#include "ctx.h"

// =================================================================================================
// Part 1. Figure out how to do 8-bit addition.

// Goal:
//
//   *out8 = (*lhs8 + *rhs8) % 256
cb_t *stamp_add8(ctx_t *ctx, bus_t out8, bus_t lhs8, bus_t rhs8) {
    // Think about how to expand what you did in `stamp_inc8` to work on multiple inputs
    //
    // In particular:
    //  - mechanically, how did you index into the table?
    //  - can you expand that to work for multiple inputs?
    // 
    volatile static _Alignas(65536) uint8_t TAB[256][256];
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            TAB[i][j] = i+j;
        }
    }

    cb_t *lhs = ctx_here(ctx);
    ctx_emit(ctx, bus(0), lhs8, 1);
    cb_t *rhs = ctx_here(ctx);
    ctx_emit(ctx, bus(0), rhs8, 1);

    cb_t *add = ctx_here(ctx);
    ctx_emit(ctx, out8, bus(&TAB), 1);

    lhs->DST_ADDR = bus((char *) &(add->SRC_ADDR) + 1);
    rhs->DST_ADDR = bus((char *) &(add->SRC_ADDR));

    /* printk("hey! %d + %d = %d\n", 1,0, *(((uint8_t*) LHS[1]))); */

    return lhs;
}

void test_add8(dma_ch_t *dma, ctx_t *ctx) {
  volatile uint8_t out, lhs, rhs;

  cb_t *program_add8 = stamp_add8(ctx, bus(&out), bus(&lhs), bus(&rhs));
  ctx_end(ctx);
  
  uint8_t TESTS[][2] = {
    // lhs, rhs
    {0x00,0x00},
    {0x00,0x01},
    {0x01,0x00},
    {0x00,0x01},
    {0xff,0xff},
    {0xff,0x01},
    {0x47,0x56},
  };
  size_t num_tests = sizeof TESTS / sizeof *TESTS;

  for(int i=0;i<num_tests;i++) {
    enum { TIMEOUT = 1024 };
    lhs = TESTS[i][0];
    rhs = TESTS[i][1];
    uint8_t expected = lhs + rhs;
    out = 0;

    dma_initiate(dma, program_add8);
    if(!dma_wait(dma,TIMEOUT)) panic("dma timed out\n");

    if(out != expected || lhs != TESTS[i][0] || rhs != TESTS[i][1])
      panic("didn't add: out=%x, lhs=%x, rhs=%x, expected=%x\n", out, lhs, rhs, expected);
  }

  output("SUCCESS!  8-bit addition works on all test inputs!\n");
}






// =================================================================================================
// Part 2. 8-bit addition with carry

cb_t *stamp_add8_carry(ctx_t *ctx, bus_t out8, bus_t carry8, bus_t lhs8, bus_t rhs8) {
    // Basically the same as stamp_add8, except you need an extra table
    // You want to write the carry to `carry8`
    volatile static _Alignas(65536) uint8_t TAB[256][256];
    volatile static _Alignas(65536) uint8_t CARRY[256][256];
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            TAB[i][j] = i+j;
            CARRY[i][j] = (i+j) > 255 ? 1 : 0;
        }
    }

    cb_t *lhs = ctx_here(ctx);
    ctx_emit(ctx, bus(0), lhs8, 1);
    cb_t *rhs = ctx_here(ctx);
    ctx_emit(ctx, bus(0), rhs8, 1);
    cb_t *add = ctx_here(ctx);
    ctx_emit(ctx, out8, bus(&TAB), 1);

    cb_t *lhs_c = ctx_here(ctx);
    ctx_emit(ctx, bus(0), lhs8, 1);
    cb_t *rhs_c = ctx_here(ctx);
    ctx_emit(ctx, bus(0), rhs8, 1);
    cb_t *carry = ctx_here(ctx);
    ctx_emit(ctx, carry8, bus(&CARRY), 1);

    lhs->DST_ADDR = bus((char *) &(add->SRC_ADDR) + 1);
    rhs->DST_ADDR = bus((char *) &(add->SRC_ADDR));
    lhs_c->DST_ADDR = bus((char *) &(carry->SRC_ADDR) + 1);
    rhs_c->DST_ADDR = bus((char *) &(carry->SRC_ADDR));

    return lhs;
}

void test_add8_carry(dma_ch_t *dma, ctx_t *ctx) {
  volatile uint8_t out, carry, lhs, rhs;

  cb_t *program_add8_carry = stamp_add8_carry(ctx, bus(&out), bus(&carry), bus(&lhs), bus(&rhs));
  ctx_end(ctx);
  
  uint8_t TESTS[][2] = {
    /// lhs, rhs
    {0x00,0x00},
    {0x01,0x00},
    {0x00,0x01},
    {0xff,0xff},
    {0xff,0x01},
    {0x47,0x56},
  };
  size_t num_tests = sizeof TESTS / sizeof *TESTS;

  for(int i=0;i<num_tests;i++) {
    enum { TIMEOUT = 1024 };
    lhs = TESTS[i][0];
    rhs = TESTS[i][1];
    uint16_t expected = (uint16_t)lhs + (uint16_t)rhs;
    out = 0;
    carry = 0;

    dma_initiate(dma, program_add8_carry);
    if(!dma_wait(dma,TIMEOUT)) panic("dma timed out\n");

    if(out != (expected&0xff) || carry != (expected>>8) || lhs != TESTS[i][0] || rhs != TESTS[i][1])
      panic("didn't add: out=%x, lhs=%x, rhs=%x, expected=%x\n", out, lhs, rhs, expected);
  }

  output("SUCCESS!  8-bit addition with carry works on all test inputs!\n");
}

// =================================================================================================
// Part 3. 32-bit addition

cb_t *stamp_add32(ctx_t *ctx, bus_t out32, bus_t lhs32, bus_t rhs32) {
    // once you have stamp_add8_carry, you can already write stamp_add32 by doing the addition you
    // learned in elementary school in base-256.
    //
    // Useful:
    //  - Ripple-Carry Adder: https://en.wikipedia.org/wiki/Adder_(electronics)
    //  - Note that stamp_add8_carry is an 8-bit half adder; you'll need to figure how to make an
    //    8-bit full adder and then chain the 8-bit full adders into a ripple-carry adder
    //  - it's helpful to remember that in addition, the carried value is always either 0 or 1,
    //    and adding together two carry values NEVER results in a carry of 1

    // let's call the output  [ o3, o2, o1, o0 ]
    // let's call the carries [ c3, c2, c1, c0 ]

    volatile static uint8_t o = 0, o1 = 0, c = 0, c1 = 0, c2=0;
    volatile static uint8_t zero = 0;

    cb_t *start = stamp_add8_carry(ctx, bus(&o), bus(&c), bus(&zero), lhs32);
    stamp_add8_carry(ctx, bus(&o1), bus(&c1), bus(&o), rhs32);
    stamp_add8_carry(ctx, bus(&c2), bus(&zero), bus(&c), bus(&c1));
    ctx_emit(ctx, out32, bus(&o1), 1);

    stamp_add8_carry(ctx, bus(&o), bus(&c), bus(&c2), bus_add(lhs32, 1));
    stamp_add8_carry(ctx, bus(&o1), bus(&c1), bus(&o), bus_add(rhs32, 1));
    stamp_add8_carry(ctx, bus(&c2), bus(&zero), bus(&c), bus(&c1));
    ctx_emit(ctx, bus_add(out32, 1), bus(&o1), 1);

    stamp_add8_carry(ctx, bus(&o), bus(&c), bus(&c2), bus_add(lhs32, 2));
    stamp_add8_carry(ctx, bus(&o1), bus(&c1), bus(&o), bus_add(rhs32, 2));
    stamp_add8_carry(ctx, bus(&c2), bus(&zero), bus(&c), bus(&c1));
    ctx_emit(ctx, bus_add(out32, 2), bus(&o1), 1);

    stamp_add8_carry(ctx, bus(&o), bus(&c), bus(&c2), bus_add(lhs32, 3));
    stamp_add8_carry(ctx, bus(&o1), bus(&c1), bus(&o), bus_add(rhs32, 3));
    stamp_add8_carry(ctx, bus(&c2), bus(&zero), bus(&c), bus(&c1));
    ctx_emit(ctx, bus_add(out32, 3), bus(&o1), 1);

    return start;
}

void test_add32(dma_ch_t *dma, ctx_t *ctx) {
  volatile uint32_t out, lhs, rhs;

  cb_t *program_add32 = stamp_add32(ctx, bus(&out), bus(&lhs), bus(&rhs));
  ctx_end(ctx);
  
  uint32_t TESTS[][2] = {
    // lhs, rhs
    {0,0},
    {1,0},
    {0,1},
    {0xffffffff,0xffffffff},
    {0xffffffff,1},
    {0x12345678,0x9abcdef0},
  };
  size_t num_tests = sizeof TESTS / sizeof *TESTS;

  for(int i=0;i<num_tests;i++) {
    enum { TIMEOUT = 1024 };
    lhs = TESTS[i][0];
    rhs = TESTS[i][1];
    uint32_t expected = lhs + rhs;
    out = 0;

    dma_initiate(dma, program_add32);
    if(!dma_wait(dma,TIMEOUT)) panic("dma timed out\n");

    if(out != expected || lhs != TESTS[i][0] || rhs != TESTS[i][1])
      panic("didn't add: out=%x, lhs=%x, rhs=%x, expected=%x\n", out, lhs, rhs, expected);
  }

  output("SUCCESS!  32-bit addition works on all test inputs!\n");
}

void notmain() {
  kmalloc_init();

  enum {
    DMA_CH = 4,
  };
  dma_ch_t *dma = dma_init(DMA_CH);

  ctx_t ctx;
  ctx_init(&ctx, 1024);

  test_add8(dma, &ctx);
  test_add8_carry(dma, &ctx);
  test_add32(dma, &ctx);

  output("SUCCESS!  All parts passed!\n");
}
