// do a raw dump of all the values for a given keypress. 
//
// this is useful for seeing how your remote behaves.
#include "rpi.h"

// simple-minded log of timed reads (value, usec)
// you should probably write your own.
enum { TR_MAX = 255 };
typedef struct timed_reads {
    unsigned n;
    struct timed_read { 
        uint32_t usec;
        uint32_t v;
    } r[TR_MAX];
} tr_t;

// initialize and return.
static inline tr_t tr_mk(void) {
    return (tr_t){};
}

// return timed read element at <i>: null if none.
struct timed_read *tr_elem(tr_t *t, unsigned i) {
    if(i >= t->n)
        return 0;
    return &t->r[i];
}

// FIFO: add entry to timed read log <l>
static void tr_push(tr_t *l, uint32_t v, uint32_t usec) {
    if(l->n >= TR_MAX)
        panic("too many entries!\n");
    let e = &l->r[l->n++];
    e->usec = usec;
    e->v = v;
}

// loop while(gpio_read(pin) == v) until either:
//   1. the pin changes: return the number of usec passed.
//   2. <timeout> is exceeded: return 0.
static uint32_t read_while_eq(int pin, int v, unsigned timeout) {
    unsigned start = timer_get_usec_raw();
    while(1) {
        // we add +1 to make sure always return != 0
        if(gpio_read(pin) != v)
            return timer_get_usec_raw() - start + 1;
        // if timeout, return 0.
        if((timer_get_usec_raw() - start) >= timeout)
            return 0;
    }
}

void notmain(void) {
    enum { 
        pin = 21,         // input pin: "S" on IR
        N = 10,             // total readings
        timeout = 40000,    // timeout in usec
     };

    gpio_set_input(pin);
    // IR goes to 0 when there is signal.
    // We use a pullup to make sure no signal = 1
    // for sure.
    gpio_set_pullup(pin);     

    // if this fails, your hardware isn't hooked up right
    assert(gpio_read(pin) == 1);

    output("will try to do a raw dump of %d readings\n", N);
    output("  recall readings are \"reversed\":\n");
    output("    no reading ==> gpio_read(pin)=1\n");
    output("    reading    ==> gpio_read(pin)=0\n");
    for(int i = 0; i < N; i++) {
        uint32_t pin_val, t, idx;

        output("trial %d: about to read\n", i);
        tr_t l = tr_mk();

        // again: default is 1, so nothing is happening.
        while((pin_val = gpio_read(pin)) == 1)
            ;
    
        // read values until timeout
        for(idx = 0;  idx < 255; idx++) {
            // read until gpio_read(pin) != v or timeout
            if(!(t = read_while_eq(pin, pin_val, timeout))) {
                // push the timeout too for debugging.
                tr_push(&l, pin_val, timeout);
                break;
            }
            tr_push(&l, pin_val, t);
            // flip so get the next value
            pin_val = 1 - pin_val;
            assert(pin_val == gpio_read(pin));
        }

        // print them out, two at a time so it's easy to see 
        // whats going on.
        for(unsigned i = 0; i < idx; i += 2) {
            let e = tr_elem(&l, i);
            assert(e);

            output("%d: pin%d=%d: usec=%d ", i, pin, e->v, e->usec);
            // in case we don't have enough readings.
            e = tr_elem(&l, i+1);
            if(e)
                output("pin%d=%d, usec=%d", pin, e->v, e->usec);
            output("\n");
        }
    }
}
