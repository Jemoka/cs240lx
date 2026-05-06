// simple program to measure the cost of naive inlined set_on/set_off
#include "rpi.h"
#include "cycle-count.h"
#include "gpio-raw.h"

// should be 8 cycles.
__attribute__((noinline)) 
static uint32_t measure_overhead(void) {
    code_align();
    uint32_t s = cycle_cnt_read();
    uint32_t e = cycle_cnt_read();
    return e-s;
}

// used to do a crude correction for cycles 
// measured by subtracting overhead.
enum { overhead = 8 };

__attribute__((noinline)) 
static uint32_t 
measure_ndelay(uint32_t n, uint32_t compensation) {
    code_align();
    uint32_t s = cycle_cnt_read();
        delay_ncycles(s, n-compensation);
    uint32_t e = cycle_cnt_read();
    return (e-s) - overhead;
}

static long abs(long x) {
    if(x<0)
        return -x;
    return x;
}

void notmain(void) {
    enum { N = 3 };

    // measure (roughly) the overhead of measurement
    for(int i = 0; i < N; i++) {
        output("%d: uncached: cycle-count overhead %d cycles\n", 
                        i, measure_overhead());
    }

    enum { uncached_compensation = 36 };
    long err_sum = 0, cnt = 0;
    // measure how accurate n_delay is
    for(long n = 100; n < 600; n+= 100) {
        for(int i = 0; i < N; i++) {
            long got = measure_ndelay(n, uncached_compensation);
            long err = got-n;
            cnt++;
            err_sum += abs(err);
            output("%d: uncached: n-delay: got=%d exp=%d [err=%d,sum=%d]\n", 
                        i, got, n, err, err_sum);
        }
    }
    output("uncached average absolute error = %d\n", err_sum/cnt);

    caches_enable();

    // measure (roughly) the overhead of measurement
    for(int i = 0; i < N; i++) {
        output("%d: cached: cycle-count overhead %d cycles\n", 
                        i, measure_overhead());
    }

    enum { cached_compensation = 20 };
    cnt = err_sum = 0;
    // measure how accurate n_delay is
    for(long n = 100; n < 600; n+= 100) {
        for(int i = 0; i < N; i++) {
            long got = measure_ndelay(n, cached_compensation);
            long err = got-n;
            err_sum += abs(err);
            cnt++;
            output("%d: cached: n-delay: got=%d exp=%d [err=%d,sum=%d]\n", 
                        i, got, n, err, err_sum);
        }
    }
    output("cached average absolute error = %d\n", err_sum/cnt);

    caches_enable();
}
