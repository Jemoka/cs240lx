// After 3-dma-blink, the only thing stopping us from doing the entire final for loop in DMA is that
// we couldn't do the `for(int i=0;i<N;i++)` in DMA. However, we just learned to do addition, so
// the only thing left is to figure out branching :)

#include "rpi.h"
#include "dma-impl.h"
#include "ctx.h"

#define KB(x) ((x)*1024)

enum { GPIO_BASE  = 0x20200000 };


static volatile _Alignas(256) uint8_t EXAMPLE[256];
static volatile _Alignas(256) uint8_t RECURSE[256];


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

static uint32_t p;

cb_t *stamp_blink(ctx_t *ctx, unsigned pin, unsigned delay) {
    static uint32_t offset = 10000;
    static volatile uint8_t one = 1;

    p = (0b1 << pin);
    cb_t *start = ctx_here(ctx);
    ctx_emit(ctx, bus((uint32_t *) (GPIO_BASE + 0x1c)), bus(&p), 1);
    cb_t *a = ctx_here(ctx);
    ctx_emit(ctx, bus(&one), bus(&one), KB(offset));
    a->TI = 0;
    ctx_emit(ctx, bus((uint32_t *) (GPIO_BASE + 0x28)), bus(&p), 1);
    a = ctx_here(ctx);
    ctx_emit(ctx, bus(&one), bus(&one), KB(offset));
    a->TI = 0;
    return start;
}

cb_t *dma_blink(ctx_t *ctx, unsigned pin, unsigned delay, bus_t n_times) {
    // NOTE: n_times is a pointer! can't unroll

    static volatile uint8_t o;

    volatile static _Alignas(256) uint8_t RECURSE[256];

    // at 0, we jump to nothing; all other values we jump back to the top of the chain
    cb_t *top = ctx_here(ctx);

    static uint32_t zero = 0;
    bus_t chicken = bus(&zero);
    for (int i = 1; i < 256; i++) {
        RECURSE[i] = 0;
    }
    RECURSE[0] = 4;

    // blink
    stamp_blink(ctx, pin, delay);

    /* (void) RECURSE; */


    // subtract 1 from n_timse
    stamp_dec8(ctx, bus(&o), n_times);
    ctx_emit(ctx, n_times, bus(&o), 1);

    // copy the offset into the jump
    cb_t *overlay = ctx_here(ctx);
    ctx_emit(ctx, bus(0), n_times, 1);

    // then copy the jump table entry corresponding to the value of n_times to the NEXT_CB of the last block in the chain
    cb_t *jump = ctx_here(ctx);
    ctx_emit(ctx, bus(0), bus(&RECURSE), 1);

    overlay->DST_ADDR = bus(&(jump->SRC_ADDR));

    cb_t *dest = ctx_here(ctx);
    ctx_emit(ctx, bus(0), bus(&chicken), 0);

    cb_t *tgt = ctx_here(ctx);
    ctx_emit(ctx, bus(&o), bus(&o), 0);

    jump->DST_ADDR = bus(&dest->TXFR_LEN);
    dest->DST_ADDR = bus(&(tgt->NEXT_CB));

    tgt->NEXT_CB = bus(top);

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
    return top;
}

void dma_delay_ms(dma_ch_t *dma, unsigned ms) {
    uint32_t offset = 10000;
    void *ptr = kmalloc(0);

    uint32_t cycle_count_start = timer_get_usec();
    cb_t a = cb_mk(bus(ptr), bus(ptr), KB(offset));
    a.TI |= (0x1f << WAITS_OFFSET);
    a.TI |= (0b1 << 26);
    a.TI |= (0b1 << 3);
    dma_run(dma, &a, 1000000000);
    uint32_t cycle_count_stop = timer_get_usec();

    printk("delay_ms: requested %d ms, actual %d ms\n", ms, (cycle_count_stop - cycle_count_start)/1000);
}

void notmain() {
  kmalloc_init();

  enum { DMA_CH = 4 };
  dma_ch_t *dma = dma_init(DMA_CH);

  ctx_t ctx;
  ctx_init(&ctx, 1024);

  enum { pin = 27, delay = 500 };
  volatile uint8_t N = 5;
  /* cb_t *program_blink = dma_blink(&ctx, pin, delay, bus(&N)); */

  /* dma_delay_ms(dma, 1000); */
  /* dma_delay_ms(dma, 1000); */
  /* dma_delay_ms(dma, 1000); */
  /* dma_delay_ms(dma, 1000); */
  gpio_set_output(pin);
  cb_t *program_blink = dma_blink(&ctx, pin, delay, bus(&N));
  dma_initiate(dma, program_blink);

  enum { TIMEOUT_MICROS = 10 * 1000 * 10000 };
  uint32_t t_start = timer_get_usec();
  while(!dma_done(dma)) {
    if ((timer_get_usec() - t_start) >= TIMEOUT_MICROS)
      panic("dma timed out after %d seconds!\n", TIMEOUT_MICROS / 1000 / 1000);
  }
}
