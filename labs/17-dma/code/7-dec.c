// Starting on computation:
// use DMA to increment an 8-bit number

#include "rpi.h"
#include "dma-impl.h"
#include "ctx.h"

// Goal: a DMA chain that will do the following:
//
//   *out = (*in + 1) % 256
//
// <in> and <out> point to some locations in memory
//
// NOTES:
//  - it's called <inc8> because it increments an 8-bit value,
//    not because it adds 8 to a value
cb_t *stamp_dec8(ctx_t *ctx, bus_t out, bus_t in) {
  // Force EXAMPLE to be 256-byte-aligned.
  static volatile _Alignas(256) uint8_t EXAMPLE[256];

  for (int i = 0; i < 256; i++) {
      EXAMPLE[i] = i-1;
  }
  cb_t *inc8_first_cb_addr = ctx_here(ctx);
  ctx_emit(ctx, bus(0), in, 1);
  cb_t *inc_actual_block_addr = ctx_here(ctx);
  ctx_emit(ctx, out, bus(&EXAMPLE), 1);
  inc8_first_cb_addr->DST_ADDR = bus(&(inc_actual_block_addr->SRC_ADDR));
  return inc8_first_cb_addr;
}


void notmain() {
    kmalloc_init();

    enum {
        DMA_CH = 4,
    };
    dma_ch_t *dma = dma_init(DMA_CH);

    // Allocate an arena of 1024 blocks (if using ctx)
    ctx_t ctx;
    ctx_init(&ctx, 1024);

    volatile uint8_t out, in;
    cb_t *program_inc8 = stamp_dec8(&ctx, bus(&out), bus(&in));
    // set the NEXT_CB of the last block to 0
    // drop if not using ctx_*
    ctx_end(&ctx);


    for (int i = 0; i < 256; i++) {
        enum {
            TIMEOUT = 1024,
        };
        out = 0;
        in = i;
        dma_initiate(dma, program_inc8);
        if (!dma_wait(dma, TIMEOUT))
            panic("dma timed out\n");

        if (out != (uint8_t)(in -1) || in != i)
            panic("didn't decrement: out=%x, in=%x expected=%x\n", out, in, (in + 1)%256);
    }

    /* output("SUCCESS!  8-bit increment works on all inputs!\n"); */
}
