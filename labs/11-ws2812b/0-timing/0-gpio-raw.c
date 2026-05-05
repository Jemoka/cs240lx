// simple program to measure the cost of naive inlined set_on/set_off
#include "rpi.h"
#include "cycle-count.h"
#include "gpio-raw.h"

// sanity check that loopback works.
static void check_loopback(uint32_t out, uint32_t in) {
    gpio_set_on(out);
    if(gpio_read(in) != 1)
        panic("missing loopback b/n pin=%d and pin=%d?\n",in,out);
    gpio_set_off(out);
    if(gpio_read(in) != 0)
        panic("missing loopback b/n pin=%d and pin=%d?\n",in,out);

    gpio_set_on_raw(out);
    if(gpio_read(in) != 1)
        panic("missing loopback b/n pin=%d and pin=%d?\n",in,out);
    gpio_set_off_raw(out);
    if(gpio_read(in) != 0)
        panic("missing loopback b/n pin=%d and pin=%d?\n",in,out);
}

// should be 8 cycles.
__attribute__((noinline)) 
static uint32_t measure_overhead(void) {
    code_align();
    uint32_t s = cycle_cnt_read();
    uint32_t e = cycle_cnt_read();
    return e-s;
}

// Q: "why so expensive?"  [hint: check the .list]
__attribute__((noinline)) 
static uint32_t measure_gpio_set_on(unsigned pin) {
    code_align();
    uint32_t s = cycle_cnt_read();
    gpio_set_on_raw(pin);
    uint32_t e = cycle_cnt_read();
    return e-s;
}

// Q: "why so expensive?"  [hint: check the .list]
__attribute__((noinline)) 
static uint32_t measure_gpio_set_off(unsigned pin) {
    code_align();
    uint32_t s = cycle_cnt_read();
    gpio_set_off_raw(pin);
    uint32_t e = cycle_cnt_read();
    return e-s;
}

void notmain(void) {
    enum { in = 27, out = 26, N = 3 };

    // setup pins and check that loopback works.
    gpio_set_output(out);
    gpio_set_input(in);
    check_loopback(out,in);

    // measure basic operations w/o cache on.

    // measure (roughly) the overhead of measurement
    for(int i = 0; i < N; i++) {
        output("%d: uncached: cycle-count overhead %d cycles\n", 
                        i, measure_overhead());
    }
    // to do a set_on
    for(int i = 0; i < N; i++) {
        output("%d: uncached: gpio_set_on = %d cycles\n", 
                        i, measure_gpio_set_on(out));
    }
    // to do a set_off
    for(int i = 0; i < N; i++) {
        output("%d: uncached: gpio_set_off = %d cycles\n", 
                        i, measure_gpio_set_off(out));
    }


    // turn caches on: doesn't matter much for this!
    caches_enable();

    // same basic operations w/ cache on.
    for(int i = 0; i < N; i++) {
        output("%d: cached: cycle-count overhead %d cycles\n", 
                        i, measure_overhead());
    }
    for(int i = 0; i < N; i++) {
        output("%d: cached: gpio_set_on = %d cycles\n", 
                        i, measure_gpio_set_on(out));
    }
    for(int i = 0; i < N; i++) {
        output("%d: uncached: gpio_set_off = %d cycles\n", 
                        i, measure_gpio_set_off(out));
    }
}
