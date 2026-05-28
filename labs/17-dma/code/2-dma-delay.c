// Goal: make a DMA chain that takes a certain amount of time to execute.
//
// We'll want this for part 3, because we'll be replacing the `delay_ms`
// function with another DMA operation.

#include "rpi.h"
#include "dma-impl.h"
#include "cycle-count.h"

// The amount of time it takes a DMA transfer to run is roughly correlated
// with the number of bytes transferred.
//
// Goal: figure out what that relationship is, and figure out a way
// to get a DMA chain that waits a certain amount of time (e.g. N milliseconds)
//
// you can do this however you want.  Probably what
// you should do is look at the TI register (p50) and
// play around with the different fields to see how
// to (1) slow down a transfer and (2) reduce memory
// traffic.
// 
// Useful: 
//   - delay: WAITS bits(21:25)
//   - wide bursts: bit(26)
//   - ignore src or dst
//   - don't increment source (bit(8)
//   - don't increment dst (bit(4))
//    - wait for response (bit(3))
//  - some others: i didn't mess around too much.
//
// Also useful: 
//   - trim out some of the fat for initiation and wait.
//
// NOTE: 
//   1. you probably don't want to increment source and dst.
//   2. to reduce bus traffic it's good to either ignore
//      dst or src (but not both)
//   3. I couldn't get delays to add enough so I copied bytes
//      too.
//
static uint32_t measure_block_exec_time(dma_ch_t *dma, uint32_t byte_count) {
    // Generate a DMA chain that copies `byte_count` bytes; use the tricks
    // noted above to figure out how to get a DMA chain that takes the
    // appropriate amount of time.
    //
    // This function should return the number of cycles it takes the chain to
    // run.
    todo("implement\n");
}

void notmain(void) {
    enum { dma_ch = 4 };
    let dma = dma_init(dma_ch);

    cycle_cnt_init();

    // First: establish a baseline measurement, because DMA startup/teardown
    // also has time costs.
    // 
    // baseline was around ~1500 cycles

    // warmup
    output("first run = %d\n", measure_block_exec_time(dma, 4));
    output("second run = %d\n", measure_block_exec_time(dma, 4));
    let baseline = measure_block_exec_time(dma, 4);
    output("baseline=%d\n", baseline);

    // measure for different inputs to `byte_count`

    for(unsigned i = 1, n = 4; n < 512; n += 4, i++) {
        output("running DMA with nbytes = %d\n", n);
        let c = measure_block_exec_time(dma,n);
        output("copied %d bytes took %d cycles, incremental=%d\n", 
            n, 
            c, 
            (c-baseline)/i);
    }
}
