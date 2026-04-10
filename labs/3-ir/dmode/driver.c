#include "rpi.h"
#include "buttons.h"

enum {
    PIN = 21,         // input pin: "S" on IR
    TIMO = 40000,     // timeout in usec
    GAP = 599         // gap epsilon
};

typedef enum {
    TIMEOUT = 0,    // timeout reached
    SIXHUNDRED = 600,    // waiting for pin to be high for 600 usec
    SIXTEENHUNDRED = 1600,   // waiting for pin to be high for 1600 usec
    FOURTYFIVEHUNDRED = 4500,   // waiting for pin to be high for 4500 usec
    NINETYHUNDRED = 9000,   // waiting for pin to be high for 9000 usec
} State;

typedef enum {
  HEADER = 3, // header signal: 9000 high, 4500 low
  BIT0 = 0,   // bit 0 signal: 600 high, 600 low
  BIT1 = 1,   // bit 1 signal: 600 high, 1600 low
  FOOTER = 2, // footer signal: 600 high, timeout low
} Signal;

///////////////////////////

static inline int abs(int v) {
    return v < 0 ? -v : v;
}
  

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

///////////////////////////

static State cast_state(int time) {
    if (abs(time - SIXHUNDRED) < GAP) {
        return SIXHUNDRED;
    } else if (abs(time - SIXTEENHUNDRED) < GAP) {
        return SIXTEENHUNDRED;
    } else if (abs(time - FOURTYFIVEHUNDRED) < GAP) {
        return FOURTYFIVEHUNDRED;
    } else if (abs(time - NINETYHUNDRED) < GAP) {
        return NINETYHUNDRED;
    } else {
        return TIMEOUT;
    }
}

void print_state(State s) {
    switch(s) {
    case TIMEOUT:
        printk("TIMEOUT\n");
        break;
    case SIXHUNDRED:
        printk("[600]\n");
        break;
    case SIXTEENHUNDRED:
        printk("[1600]\n");
        break;
    case FOURTYFIVEHUNDRED:
        printk("[4500]\n");
        break;
    case NINETYHUNDRED:
        printk("[9000]\n");
        break;
    }
}

///////////////////////////

static Signal decode(State high, State low) {
    if (high == SIXHUNDRED && low == SIXHUNDRED) {
        return BIT0;
    } else if (high == SIXHUNDRED && low == SIXTEENHUNDRED) {
        return BIT1;
    } else if (high == SIXHUNDRED && low == TIMEOUT) {
        return FOOTER;
    } else if (high == NINETYHUNDRED && low == FOURTYFIVEHUNDRED) {
        return HEADER;
    } else {
        return FOOTER;
    }
}

void print_signal(Signal s) {
    switch(s) {
    case HEADER:
        printk("\nHEADER\n");
        break;
    case BIT0:
        printk("0");
        break;
    case BIT1:
        printk("1");
        break;
    case FOOTER:
        printk("\nFOOTER\n\n");
        break;
    }
}

///////////////////////////

void notmain(void) {

    // set pullup to get ready for read
    gpio_set_input(PIN);
    gpio_set_pullup(PIN);

    // block until something happens
    while (1) {
        uint32_t pin_val, time;
        while((pin_val = gpio_read(PIN)) == 1) {};

        uint32_t times[1000];
        uint32_t *time_ptr = times;
        *time_ptr++ = read_while_eq(PIN, pin_val, TIMO);

        while (*(time_ptr - 1) != 0) {
            pin_val = 1 - pin_val;
            *time_ptr++ = read_while_eq(PIN, pin_val, TIMO);
        }

        // print states
        uint32_t n_times = time_ptr - times;
        /* for (int i = 0; i < n_times; i++) { */
        /*     print_state(cast_state(times[i])); */
        /* } */
        /* printk("Got %u timings\n", n_times); */

        // decode signals
        uint32_t out = 0;
        uint32_t offset = 0;

        for (int i = 0; i < n_times - 1; i+=2) {
            Signal s = decode(cast_state(times[i]), cast_state(times[i+1]));
            print_signal(s);
            if (s == BIT1) {
                out |= (1 << (32-offset));
            } else if (s == BIT0) {
                out &= ~(1 << (32-offset));
            }
            offset++;
            if (s == FOOTER) {
                break;
            }
        }

        printk("Got signal: %s\n", button_to_str(out));
    }

}

