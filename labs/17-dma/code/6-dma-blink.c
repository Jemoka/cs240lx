// After 3-dma-blink, the only thing stopping us from doing the entire final for loop in DMA is that
// we couldn't do the `for(int i=0;i<N;i++)` in DMA. However, we just learned to do addition, so
// the only thing left is to figure out branching :)

#include "rpi.h"
#include "dma-impl.h"
#include "ctx.h"

// Feel free to put your own helpers in here :)

cb_t *dma_blink(ctx_t *ctx, unsigned pin, unsigned delay, bus_t n_times) {
  // NOTE: n_times is a pointer! can't unroll

  // Goal: should be equivalent to the for(i=0;i<N;i++) { on ; delay ; off ; delay;  }
  //
  // It's useful to break this down to pseudo-assembly:
  //
  // loop:
  //     if i = N { goto end } <- this is the part you need to figure out
  //     i = i + 1
  //     on
  //     delay
  //     off
  //     delay
  //     goto loop <- you can use NEXT_CB to get this effect
  // end:
  //     exit <- this can be a no-op control block that has NEXT_CB=0
  // 
  // Useful for conditionals:
  //  - basic idea is https://en.wikipedia.org/wiki/Branch_table
  //  - similar to what you've done before, except instead of manipulating the SRC_ADDR of another
  //    block, you're manipulating the NEXT_CB!
  //  - you can assume n_times<256 if it helps
  //  - you might need to use multiple tables
  todo("implement me!");
}

void notmain() {
  kmalloc_init();

  enum { DMA_CH = 4 };
  dma_ch_t *dma = dma_init(DMA_CH);

  ctx_t ctx;
  ctx_init(&ctx, 1024);

  enum { pin = 27, delay = 500 };
  volatile uint8_t N = 5;
  cb_t *program_blink = dma_blink(&ctx, pin, delay, bus(&N));

  dma_initiate(dma, program_blink);
  // should take about 5s; round it up to 10
  enum { TIMEOUT_MICROS = 10 * 1000 * 1000 };
  uint32_t t_start = timer_get_usec();
  while(!dma_done(dma)) {
    if ((timer_get_usec() - t_start) >= TIMEOUT_MICROS)
      panic("dma timed out after %d seconds!\n", TIMEOUT_MICROS / 1000 / 1000);
  }
}
