// measure roughly how accurate write_0 and write_1 are.
//  - uses the same polling approach as the previous tests.  
//  - for more accuracy, use a logic analyzer, SMI, DMA, or do 
//    the first lab in 340lx.
#include "rpi.h"
#include "cycle-count.h"
#include "gpio-raw.h"

// r/pi A+ default CPU cycles per sec: 700MHz (700 million cycles per second)
#define MHz 700UL

// convert nanoseconds to pi cycles: you need to modify this if you change
// the pi's clock speed.
#define ns_to_cycles(x) (unsigned) (((unsigned)x * 7UL) / 10UL )

enum {
    X1=630,
    X2=245,
    X3=245,
    X4=630
};

enum { overhead = 0 };

__attribute__((noinline)) 
static uint32_t 
measure_write_1(uint32_t pin, uint32_t ncycles) {
    code_align();
    uint32_t s = cycle_cnt_read();
        write_1(pin, ncycles);
    uint32_t e = cycle_cnt_read();
    return (e-s) - overhead;
}

__attribute__((noinline)) 
static uint32_t 
measure_write_0(uint32_t pin, uint32_t ncycles) {
    code_align();
    uint32_t s = cycle_cnt_read();
        write_0(pin, ncycles);
    uint32_t e = cycle_cnt_read();
    return (e-s) - overhead;
}

static long abs(long x) {
    if(x<0)
        return -x;
    return x;
}

void notmain(void) {
    enum { pin = 26, N = 3 };

    caches_enable();
    long tot = 0;
    for(int i = 0; i < N; i++) {
        uint32_t got,exp;
        long err;

        got = measure_write_1(pin, exp = X1);
        output("write_1 got=%d, exp=%d, err=%d\n", got,exp, err=exp-got);
        tot += abs(err);

        got = measure_write_1(pin, exp = X2);
        output("write_1 got=%d, exp=%d, err=%d\n", got,exp, err=exp-got);
        tot += abs(err);

        got = measure_write_0(pin, exp = X3);
        output("write_0 got=%d, exp=%d, err=%d\n", got,exp, err=exp-got);
        tot += abs(err);

        got = measure_write_0(pin, exp = X4);
        output("write_0 got=%d, exp=%d, err=%d\n", got,exp, err=exp-got);
        tot += abs(err);
    }
    output("tot erro=%d, ave=%d cycles\n", tot, tot/(N*4));

}
