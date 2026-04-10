#include "rpi.h"

enum GPIOAddrs {
    IRQ_PENDING_2 = 0x2000B208, // 7.5 page 112
    IRQ_ENABLE_2 = 0x2000B214, // 7.5 page 112

    EVENT_STATUS_0 = 0x20200040,
    RISING_EDGE_BASE_0 = 0x2020004C,
    FALLING_EDGE_BASE_0 = 0x20200058,
};

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
//
// also have to enable GPIO interrupts at all in <IRQ_Enable_2>
void gpio_int_rising_edge(unsigned pin) {
    if(pin>=32)
        return;
    dev_barrier();
    OR32(RISING_EDGE_BASE_0, (0b1 << pin));
    dev_barrier();
    PUT32(IRQ_ENABLE_2, ((0b1) << (49-32)));
    dev_barrier();
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
//
// also have to enable GPIO interrupts at all in <IRQ_Enable_2>
void gpio_int_falling_edge(unsigned pin) {
    if(pin>=32)
        return;
    dev_barrier();
    OR32(FALLING_EDGE_BASE_0, (0b1 << pin));
    dev_barrier();
    PUT32(IRQ_ENABLE_2, ((0b1) << (49-32)));
    dev_barrier();
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if(pin>=32)
        return 0;
    dev_barrier();
    int has = GET32(EVENT_STATUS_0) & (0b1 << pin);
    dev_barrier();
    if (has) { return 1;  }
    else { return 0; }
    
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if(pin>=32)
        return;
    dev_barrier();
    PUT32(EVENT_STATUS_0, (0b1 << pin));
    dev_barrier();
}
