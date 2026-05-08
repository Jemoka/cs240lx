#ifndef __timed_h__
#define __timed_h__

#include "cycle-count.h"

#ifndef GPIO_BASE
#   define GPIO_BASE 0x20200000
#endif
#define GPIO_LEV0 (void*)(GPIO_BASE + 0x34)
#define GPIO_CLR0 (void*)(GPIO_BASE + 0x28)
#define GPIO_SET0 (void*)(GPIO_BASE + 0x1C)

////////////////////////////////////////////////////////////

static inline void put32_raw(volatile void * addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}
static inline void PUT32_raw(uint32_t addr, uint32_t val) {
    put32_raw((void*)addr,val);
}
static inline uint32_t get32_raw(volatile void * addr) {
    return *(volatile uint32_t *)addr;
}
static inline uint32_t GET32_raw(uint32_t addr) {
    return get32_raw((void*)addr);
}

////////////////////////////////////////////////////////////

// NOTE: only works for [0....31]
static inline void timed_gpio_set_on(unsigned pin) {
    put32_raw((volatile void*) GPIO_SET0, (0b1 << pin));
}
static inline void timed_gpio_set_off(unsigned pin) {
    put32_raw((volatile void*) GPIO_CLR0, (0b1 << pin));
}
static inline int32_t timed_gpio_read(unsigned pin) {
    uint32_t v;
    v = get32_raw((volatile void*) GPIO_LEV0) & (0b1 << pin);
    return (v != 0);
}

////////////////////////////////////////////////////////////

static inline uint32_t delay_ncycles(unsigned s, unsigned n) {
    uint32_t e;
    do {
        e = cycle_cnt_read();
    } while((e - s) < n);
    return e;
}

////////////////////////////////////////////////////////////

#define MHz 700UL
#define CYCLE_CORRECTION 100 // in case cycle counting measurement incurrs cost
#define ns_to_cycles(x) (unsigned) (((unsigned)x * 7UL) / 10UL )

////////////////////////////////////////////////////////////

static inline void write_1(unsigned pin, unsigned ns) {
    timed_gpio_set_on(pin);
    delay_ncycles(cycle_cnt_read(), ns_to_cycles(ns)-CYCLE_CORRECTION);
}

static inline void write_0(unsigned pin, unsigned ns) {
    timed_gpio_set_off(pin);
    delay_ncycles(cycle_cnt_read(), ns_to_cycles(ns)-CYCLE_CORRECTION);
}

#endif
