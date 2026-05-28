// an example of how to work with the ctx API

#include "ctx.h"
#include "rpi.h"
#include "dma-impl.h"

void notmain(void) {
  enum { DMA_CH = 4 };
  dma_ch_t *dma = dma_init(DMA_CH);

  volatile uint32_t dst,
                    one = 1, two = 2, three = 3, four = 4;

  ctx_t ctx;
  // allocate a contiguous segment of memory with space for 1024 control blocks
  ctx_init(&ctx, 1024);

  // get pointer where next control block will be created
  cb_t *first_cb = ctx_here(&ctx);

  // emit a control block (at `first_cb`) that copies 4 bytes from `one` to `dst`.
  // 
  // note that this control block's NEXT_CB pointer will automatically point to
  // the control block below that sets dst to 2
  ctx_emit(&ctx, bus(&dst), bus(&one), 4);

  // emit another control block; set dst to 2
  ctx_emit(&ctx, bus(&dst), bus(&two), 4);
  
  // get pointer to the last control block that was created (the one that sets
  // dst to 2)
  cb_t *second_cb = ctx_last(&ctx);
  
  // Next, we're going to show how to set the NEXT_CB with the ctx API.
  // Specifically, we're going to skip over the control block that sets dst to 3.
  
  // will be skipped over!
  bus_t third_cb_bus = bus(ctx_here(&ctx));
  ctx_emit(&ctx, bus(&dst), bus(&three), 4);

  // ctx_label(c) is a shorthand for bus(ctx_here(c))
  bus_t fourth_cb_bus = ctx_label(&ctx);
  ctx_emit(&ctx, bus(&dst), bus(&four), 4);

  // go back and modify the second control block's NEXT_CB so that instead of
  // pointing to the third control block, it points to the fourth instead!
  assert(bus_eq(second_cb->NEXT_CB, third_cb_bus));
  second_cb->NEXT_CB = fourth_cb_bus;

  // finally, we always need to end our DMA chains with NEXT_CB=0
  // The following are all equivalent ways to do this:
  ctx_last(&ctx)->NEXT_CB = (bus_t){0};
  ctx_branch(&ctx, (bus_t){0}); // <- this one is actually quite useful for
                                //    doing control flow in general
  ctx_end(&ctx);
}
